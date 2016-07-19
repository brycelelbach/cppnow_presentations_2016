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
        auto it = listeners.end();
        do {
            it--;
            if (*it == listener) {
                listeners.erase(it);
                return true;
            }
        } while (it != listeners.begin());
        return false;
    }

private:
    void notifyListeners() {
        for (auto & listener : listeners)
            listener->fooChanged(this);
    }

    std::vector<Listener*> listeners;
};


