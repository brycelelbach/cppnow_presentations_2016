// example ObserverPattern.cpp
// Tony Van Eerd


#include <vector>
#include <algorithm>
#include <iostream>

#include <mutex>

#include "ThreadsafeFoo9.h"

std::mutex outMu;

template <typename T0, typename T1, typename T2>
void out(T0 const & t0, T1 const & t1, T2 const & t2)
{
    std::lock_guard<std::mutex> lock(outMu);
    std::cout << t0 << ' ' << t1 << ' ' << t2 << '\n';
}
template <typename T0, typename T1>
void out(T0 const & t0, T1 const & t1)
{
    std::lock_guard<std::mutex> lock(outMu);
    std::cout << t0 << ' ' << t1 << '\n';
}
template <typename T0>
void out(T0 const & t0)
{
    std::lock_guard<std::mutex> lock(outMu);
    std::cout << t0 << '\n';
}
template <typename T0, typename T1>
void lout(T0 const & t0, T1 const & t1)
{
    out(">", t0, t1);
}
template <typename T0>
void lout(T0 const & t0)
{
    out(">", t0);
}



void test_addremove()
{
    out("--------- simple add, set, remove, set");
    Foo foo;

    struct MyListener : Foo::Listener {
        void fooChanged(Foo * foo) override {
            lout(foo->get());
        }
    };
    MyListener mine;
    out("add one");
    foo.addListener(&mine);
    out("foo = Hello world");
    foo.set("Hello world");
    out("remove one");
    foo.removeListener(&mine);
    out("foo = goodbye");
    foo.set("goodbye"); // should not appear
}



void test_order()
{
    out("--------- order");
    Foo foo;

    struct MyListener : Foo::Listener {
        std::string name;
        MyListener(std::string name) : name(name) { }
        void fooChanged(Foo * foo) override {
            lout(foo->get(), name);
        }
    };
    out("add alice, then bob, then set foo");
    MyListener alice("alice");
    foo.addListener(&alice);
    MyListener bob("bob");
    foo.addListener(&bob);
    foo.set("hello");
}



void test_stackedaddremove()
{
    out("--------- stacked add/remove and more");
    Foo foo;

    struct MyListener : Foo::Listener {
        std::string name;
        MyListener(std::string name) : name(name) { }
        void fooChanged(Foo * foo) override {
            lout(foo->get(), name);
        }
    };
    MyListener alice("alice");
    out("add alice");
    foo.addListener(&alice);
    foo.set("one listener");

    MyListener bob("bob");
    out("add bob");
    foo.addListener(&bob);
    foo.set("two listeners");
    
    out("add alice - again!");
    foo.addListener(&alice);
    foo.set("three listeners");
    
    out("remove alice --- which one?");
    foo.removeListener(&alice);
    foo.set("two listeners");
    
    out("remove alice (not at end)");
    foo.removeListener(&alice);
    foo.set("one listener");
    
    out("remove alice (too many times)");
    foo.removeListener(&alice);
    //foo.set("one listener") - this doesn't notify anyone! value didn't change!
    foo.set("one listener still");

    foo.removeListener(&bob);
    foo.set("zero listeners");
    out("remove bob again - on empty list");
    foo.removeListener(&bob);  // listener not found - should not crash
    foo.set("zero listeners");
}



void test_recursive_remove()
{
    out("---------  recursive remove");
    Foo foo;

    struct OneShotListener : Foo::Listener {
        Foo & foo;
        OneShotListener(Foo & foo) : foo(foo) {
            startListening();
        }
        ~OneShotListener() {
            stopListening();
        }
        void fooChanged(Foo * foo) override {
            lout(foo->get());
            stopListening();
        }
        void startListening() { foo.addListener(this); }
        void stopListening() { foo.removeListener(this); }
    };
    OneShotListener oneshot(foo);
    out("add listener which will self-remove");
    out("set foo = Hello");
    foo.set("Hello");
    out("set foo = world");
    foo.set("world");
    out("no listeners!");
}


void test_recursive_add()
{
    out("---------  recursive add");
    Foo foo;

    struct Expect : Foo::Listener {
        std::string name;
        std::string expect;
        Expect * next;
        Expect(std::string name, std::string expect, Expect * next = 0)
            : name(name), expect(expect), next(next)
        {
        }
        void fooChanged(Foo * foo) override {
            lout(name, foo->get());
            if (foo->get() == expect) {
                foo->addListener(next);
                foo->removeListener(this);
            }
        }
    };
    Expect world("bob", "world");
    Expect hello("alice", "Hello", &world);
    out("add alice, who will remove self and add bob");
    foo.addListener(&hello);
    out("set foo = Hello");
    foo.set("Hello");
    out("set foo = world");
    foo.set("world");
}


void test_recursive_add_with_bystanders()
{
    out("---------  recursive add with charlie");
    Foo foo;

    struct Expect : Foo::Listener {
        std::string name;
        std::string expect;
        Expect * next;
        Expect(std::string name, std::string expect, Expect * next = 0)
            : name(name), expect(expect), next(next)
        {
        }
        void fooChanged(Foo * foo) override {
            lout(name, foo->get());
            if (foo->get() == expect) {
                foo->addListener(next);
                foo->removeListener(this);
            }
        }
    };
    Expect world("bob", "world");
    Expect charlie("charlie", "charlie");
    Expect hello("alice", "Hello", &world);
    out("add alice, who will remove self and add bob");
    foo.addListener(&hello);
    out("also add charlie");
    foo.addListener(&charlie);
    out("set foo = Hello");
    foo.set("Hello");
    out("set foo = world");
    foo.set("world");
}



void test_recursive_remove_other()
{
    out("---------  recursive remove other");
    Foo foo;

    struct Expect : Foo::Listener {
        char const * name;
        std::string expect;
        Expect * other;
        Expect(char const * name, std::string expect, Expect * other = 0)
            : name(name), expect(expect), other(other)
        {
        }
        void fooChanged(Foo * foo) override {
            lout(name, foo->get());
            if (foo->get() == expect && other) {
                foo->removeListener(other);
                other->name = nullptr;
            }
        }
    };

    Expect hello("alice", "Hello");
    Expect world("bob", "world", &hello);
    Expect delta("delta", "delta");
    Expect charlie("charlie", "world", &delta);

    out("add 4 listeners");
    out("bob will remove alice on world");
    out("charlie will remove delta on world");
    foo.addListener(&hello);
    foo.addListener(&world);
    foo.addListener(&charlie);
    foo.addListener(&delta);
    out("foo = test");
    foo.set("test");
    out("foo = Hello");
    foo.set("Hello");
    out("set foo = world");
    out("***");
    foo.set("world");
    out("set foo = test");
    foo.set("test");
    out("set foo = world");
    foo.set("world");
    out("set foo = test");
    foo.set("test");
}


int main(int argc, char* argv[])
{
    test_addremove();
    test_order();
    test_stackedaddremove();
    test_recursive_remove();
    test_recursive_add();
    test_recursive_add_with_bystanders();
    test_recursive_remove_other();

    void threading_tests();
    threading_tests();
}

#include "threading_tests.cpp" // never #include cpp files
