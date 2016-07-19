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
        std::vector<Listener*> temp;
        {
            LockGuard lock(mu);
            temp = listeners;
        }
        int size = (int)temp.size();
        for (int i = 0; i < size; i++)
            if (listeners[i])    // ???
                temp[i]->fooChanged(this);

        { LockGuard lock(mu);
        listeners.erase(std::remove(listeners.begin(), listeners.end(), nullptr), listeners.end());
        }
    }

    std::vector<Listener*> listeners;
};


