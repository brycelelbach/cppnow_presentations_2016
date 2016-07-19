// example ObserverPattern
// Tony Van Eerd

#include <vector>
#include <algorithm>
#include <string>

#include <mutex>

class Foo
{
    std::string s;
    std::recursive_mutex mu;
    typedef std::lock_guard<std::recursive_mutex> LockGuard;

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
        for (auto it = listeners.end();  it != listeners.begin();  )
        {
            it--;
            if (*it == listener) {
                listeners.erase(it);
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
            temp[i]->fooChanged(this);
    }

    std::vector<Listener*> listeners;
};


