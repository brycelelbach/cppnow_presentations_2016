// example ObserverPattern
// Tony Van Eerd

#include <vector>
#include <algorithm>
#include <string>

#include <mutex>
#include <thread>
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
    void notifyListeners() {
        LockGuard lock(mu);
        int size = (int)listeners.size();
        for (int i = 0; i < size; i++)
        {
            UnlockGuard<> unlock(mu);
            if (listeners[i]) {   // ???
                listeners[i]->fooChanged(this);
            }
        }
    }

    std::vector<Listener*> listeners;
};


