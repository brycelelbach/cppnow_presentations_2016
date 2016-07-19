// example ObserverPattern
// Tony Van Eerd

#include <vector>
#include <algorithm>
#include <string>

class Foo
{
    std::string s;

public:
    void set(std::string str) {
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
        listeners.push_back(listener);
    }

    bool removeListener(Listener * listener) {
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
        int size = (int)listeners.size();
        for (int i = 0; i < size; i++)
            if (listeners[i])
                listeners[i]->fooChanged(this);
        // remove any nulls from recursive removeListener calls
        listeners.erase(std::remove(listeners.begin(), listeners.end(), nullptr), listeners.end());
    }

    std::vector<Listener*> listeners;
};


