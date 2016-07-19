// example ObserverPattern
// Tony Van Eerd

#include <list>
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
        if (listeners.empty())
            return;

        auto last = --listeners.end();
        for (auto it = listeners.begin(); ;  )
        {
            auto next = ++it;
            --it;
            if (*it)
                (*it)->fooChanged(this);
            if (it == last)
                break;
            it = next;
        }
    }

    std::list<Listener*> listeners;
};


