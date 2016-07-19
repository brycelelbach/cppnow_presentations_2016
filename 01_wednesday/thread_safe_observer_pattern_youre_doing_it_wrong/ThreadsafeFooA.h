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

        void waitUntilUnused()
        {
            int use = unmarked();
            while (use) {
                mu.lock();
                mu.unlock();
                use = unmarked();
            }
        }
        void deleteSelf(bool wait)
        {
            int use = markIt();
            if (wait || use == 0) {
                waitUntilUnused();
                delete this;
            }
        }

    private:
        friend class Foo;
        void call_fooChanged(Foo * foo)
        {
            std::lock_guard<std::mutex> lock(mu);
            fooChanged(foo);
        }
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
                else if (unmarked(use) >100) {
                    int iuse = (int)use;
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
        std::mutex mu;
    };

    void addListener(Listener * listener) {
        LockGuard lock(mu);
        listeners.push_back(listener);
    }

    bool removeListener(Listener * listener, bool wait = false) {
        LockGuard lock(mu);
        for (auto it = listeners.end();  it != listeners.begin();  )
        {
            it--;
            if (*it == listener) {
                *it = nullptr;
                if (wait)
                    listener->waitUntilUnused();
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
                    temp->call_fooChanged(this);
                    sleep();
                    temp->decrUse();
                }
            }
        }
    }

    std::vector<Listener * > listeners;
};


