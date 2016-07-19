// example ObserverPattern
// Tony Van Eerd

#include <vector>
#include <algorithm>
#include <string>

#include <mutex>

class Foo
{
    std::string s;
    std::mutex mu;
    typedef std::lock_guard<std::mutex> LockGuard;

public:
    void set(std::string str) {
        LockGuard lock(mu);
        if (s != str) {
            s = str;
            notifyListeners();
        }
    }

    std::string get() { return s; }

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
        for (auto it = listeners.rbegin();  it != listeners.rend();  it++)
        {
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
            if (listeners[i])
                listeners[i]->fooChanged(this);
        // remove any nulls from recursive removeListener calls
        listeners.erase(std::remove(listeners.begin(), listeners.end(), nullptr), listeners.end());
    }

    std::vector<Listener*> listeners;
};


