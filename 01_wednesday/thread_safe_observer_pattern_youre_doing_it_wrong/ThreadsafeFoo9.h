// example ObserverPattern
// Tony Van Eerd

#include <vector>
#include <algorithm>
#include <string>

#include <mutex>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>
#include "UnlockGuard.h"

class Foo
{
    std::string s;
    std::mutex mu;
    typedef std::lock_guard<std::mutex> LockGuard;

public:
    void set(std::string str) {
        bool notify = false;
        {
            LockGuard lock(mu);
            if (s != str) {
                s = str;
                notify = true;
            }
        }
        if (notify)
            notifyListeners();
    }

    std::string get() { LockGuard lock(mu);  return s; }

    struct Listener
    {
        virtual void fooChanged(Foo * foo) = 0;

        virtual ~Listener() {}
        Listener() : useCount(0) {}

        void deleteSelf()
        {
            if (markIt() == 0)
                delete this;
        }

    private:
        friend class Foo;
        int useCountWas() { return unmarked(); }
        bool incrUse()
        {
            unsigned int use = useCount.load();
            do
            {
                if (marked(use)) // if marked for deletion, can't use it
                    return false; // ensure count doesn't increase!
            }
            while (!useCount.compare_exchange_weak(use, use+1));
            return true;
        }
        void decrUse()
        {
            unsigned int use = useCount.load();
            do
            {
                if (marked(use) && unmarked(use) <= 1)  // last user
                {
                    delete this;
                    break;
                }
            }
            while (!useCount.compare_exchange_weak(use, use-1));
        }
        static const unsigned int HIGH_BIT = 0x80000000UL;
        unsigned int unmarked(unsigned int x) { return x & ~HIGH_BIT; }
        unsigned int unmarked() { return unmarked(useCount.load()); }
        bool marked(unsigned int x) { return !!(x & HIGH_BIT); }
        bool marked() { return marked(useCount.load()); }
        unsigned int markIt() { return unmarked(useCount.fetch_or(HIGH_BIT)); } // return count

        std::atomic<unsigned int> useCount;
    };

    void addListener(Listener * listener) {
        LockGuard lock(mu);
        listeners.push_back(listener);
    }

    bool removeListener(Listener * listener) {
        LockGuard lock(mu);
        for (auto it = listeners.end();  it != listeners.begin();  )
        {
            it--;
            if (*it == listener) {
                *it = nullptr;
                return true;
            }
        }
        return false;
    }

private:
    void sleep()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    void notifyListeners() {
        LockGuard lock(mu);
        int size = (int)listeners.size();
        for (int i = 0; i < size; i++)
        {
            if (listeners[i]) {
                Listener * temp = listeners[i];
                if (temp->incrUse())
                {
        UnlockGuard<> unlock(mu);
                    temp->fooChanged(this);
                    sleep();
                    temp->decrUse();
                }
            }
        }
    }

    std::vector<Listener * > listeners;
};


