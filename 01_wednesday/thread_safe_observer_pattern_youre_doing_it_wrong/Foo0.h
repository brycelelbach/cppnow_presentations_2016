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
        s = str;
        notifyListeners();
    }
    std::string get() { return s; }

    struct Listener
    {
        virtual void fooChanged(Foo * foo) = 0;
    };

    void addListener(Listener * listener) {
        listeners.push_back(listener);
    }

    void removeListener(Listener * listener) {
        std::remove(listeners.begin(), listeners.end(), listener);
    }

private:
    void notifyListeners() {
        for (auto & listener : listeners)
            listener->fooChanged(this);
    }

    std::vector<Listener*> listeners;
};


