// example ObserverPattern.cpp
// Tony Van Eerd

#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

void test_twoleftturns()
{
    out("--------- test two left turns");

    struct MyListener : Foo::Listener {
        char const * name;
        MyListener(char const * name) : name(name) { }
        void fooChanged(Foo * foo) override {
            lout(foo->get(), name);
        }
    };
    struct Expect : Foo::Listener {
        char const * name;
        MyListener * hello;
        MyListener * world;
        Foo * foo1;
        Foo * foo2;
        Expect(char const * name, Foo * foo1, Foo * foo2, MyListener * hello, MyListener * world)
            : name(name), foo1(foo1), foo2(foo2), hello(hello), world(world)
        {
        }
        void fooChanged(Foo * foo) override {
            lout(name, foo->get());
            if (foo->get() == "hello") {
                foo1->addListener(hello);
                hello->name = "hbob";
            } else {
                foo1->removeListener(hello);//, true);
                hello->name = nullptr;
            }
            if (foo->get() == "world") {
                foo2->addListener(world);
                world->name = "walice";
            } else {
                foo2->removeListener(world);//, true);
                world->name = nullptr;
            }
        }
    };

    Foo foo1;
    Foo foo2;

    MyListener alice("walice");
    MyListener bob("hbob");
    Expect expect("expect", &foo1, &foo2, &bob, &alice);

    foo1.addListener(&expect);
    foo2.addListener(&expect);

    auto alot = [](Foo *foo, std::string name) {
        for (int i = 0; i < 10; i++) {
            out(name, "hello");
            foo->set("hello");
            out(name, "world");
            foo->set("world");
        }
    };

    std::thread one(alot, &foo1, "one");
    std::thread two(alot, &foo2, "two");

    one.join();
    two.join();
    out("done");
}

void sleep(int dur = 50)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(dur));
}

void test_deleteSelf()
{
    out("--------- test deleteSelf()");

    struct MyListener : Foo::Listener {
        std::string name;
        MyListener(std::string name) : name(name) { }
        void fooChanged(Foo * foo) override {
            lout(foo->get(), name);
        }
    };
    struct Killer : Foo::Listener {
        char const * name;
        Foo * foo;
        std::atomic<MyListener *> target;
        Killer(char const * name, Foo * foo) : name(name), foo(foo)
        {
        }
        void kill(MyListener * tar)
        {
            foo->removeListener(tar);
            //tar->deleteSelf();
        }
        void setTarget(MyListener * listener)
        {
            MyListener * old = target.exchange(listener);
            if (old)
                kill(old);
        }

        void fooChanged(Foo * foo) override {
            lout(name, foo->get());
            MyListener * tar = target.exchange(nullptr);
            if (tar)
                kill(tar);
        }
    };

    Foo foo;

    Killer killer("killer", &foo);

    foo.addListener(&killer);

    auto alot = [&killer](Foo *foo, std::string tname) {
        for (int i = 0; i < 20; i++) {
            if (tname == "one")
            {
                std::string name = "alice"; name += ('A' + i);
                MyListener * alice = new MyListener(name);
                foo->addListener(alice);
                sleep();
                killer.setTarget(alice);
            }
            out(tname, "hello");
            std::string msg = tname; msg += 'A' + i; msg += "hello";
            foo->set(msg);
            //sleep();
            out(tname, "world");
            foo->set(tname + "world");
            //sleep();
        }
    };

    std::thread one(alot, &foo, "one");
    std::thread two(alot, &foo, "two");
    std::thread thr(alot, &foo, "thr");

    one.join();
    two.join();
    thr.join();
    out("done");
}


void do_other_stuff()
{
    sleep(10);
}

// change of interface
// too lazy to update old tests
// Foo is now in namespace Hack
#include "ThreadsafeFooB.h"

namespace Hack {

void test_independent()
{
    out("--------- test independent");

    struct MyListener : Foo::Listener {
        std::string name;
        MyListener(std::string name) : name(name) { }
        void fooChanged(std::string s) override {
            lout(s, name);
        }
    };

    MyListener alice("alice");
    MyListener bob("bob");
    Foo foo;

    foo.addListener(&alice);
    foo.addListener(&bob);

    auto writing = [](Foo * foo, std::string tname) {
        for (int i = 0; i < 20; i++) {
            out(tname, "hello");
            std::string msg = tname; msg += 'A' + i; msg += "hello";
            foo->set(msg);
            //sleep();
            out(tname, "world");
            foo->set(tname + "world");
            //sleep();
        }
    };

    auto listening = [](Foo::Listener * listener) {
        listener->for_every_change();
    };

    std::atomic<bool> stuff_to_do = true;

    auto listening_detail = [&](Foo::Listener * listener) {
        while (stuff_to_do)
        {
            listener->check_for_change();
            do_other_stuff();
        }
    };

    std::thread writer(writing, &foo, "writer");

    std::thread listener1(listening, &alice);
    std::thread listener2(listening_detail, &bob);

    writer.join();
    // writing done, stop the listener threads
    alice.stop();
    stuff_to_do = false;
    listener1.join();
    listener2.join();
    out("done");
}

void test_deletion()
{
    out("--------- test deletion");

    Foo foo;

    std::atomic<bool> keepWriting = true;
    auto writing = [&keepWriting](Foo * foo, std::string tname) {
        for (int i = 0; keepWriting; i++) {
            out(tname, "hello");
            std::string msg = tname; msg += 'A' + i; msg += "hello";
            foo->set(msg);
            //sleep();
            out(tname, "world");
            foo->set(tname + "world");
            //sleep();
        }
    };

    // start writing, at first, with no listeners
    std::thread writer(writing, &foo, "writer");

    // write for a while
    sleep(50);

    // now add some listeners
    {
        struct MyListener : Foo::Listener {
            std::string name;
            MyListener(std::string name) : name(name) { }
            void fooChanged(std::string s) override {
                lout(s, name);
            }
        };

        MyListener alice("alice");
        MyListener bob("bob");

        foo.addListener(&alice);
        foo.addListener(&bob);

        auto listening = [](Foo::Listener * listener) {
            listener->for_every_change();
        };

        std::atomic<bool> stuff_to_do = true;

        auto listening_detail = [&](Foo::Listener * listener) {
            while (stuff_to_do)
            {
                listener->check_for_change();
                do_other_stuff();
            }
        };

        std::thread listener1(listening, &alice);
        std::thread listener2(listening_detail, &bob);

        // wait a while
        sleep(200);

        foo.removeListener(&alice);
        foo.removeListener(&bob);

        // stop alice and bob
        alice.stop();
        stuff_to_do = false;
        listener1.join();
        listener2.join();
        // *DESTROY* alice and bob here
    }

    // keep writing for a while
    sleep(50);
    // stop writer
    keepWriting = false;
    writer.join();

    out("done");
}

} //Hack

void threading_tests()
{
    //test_twoleftturns();
    //test_deleteSelf();
    Hack::test_independent();
    Hack::test_deletion();
}