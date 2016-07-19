// example ObserverPattern
// Tony Van Eerd

#include <mutex>

template <typename Mutex = std::mutex>
class UnlockGuard
{
public:

    explicit UnlockGuard(Mutex & mu) : mu(mu)
	{
	    mu.unlock();
	}

    ~UnlockGuard()
	{
	    mu.lock();
	}

private:
    UnlockGuard(UnlockGuard const &);//= delete;
    UnlockGuard & operator=(UnlockGuard const &);// = delete;

    Mutex & mu;
};
