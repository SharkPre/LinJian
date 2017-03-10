#ifndef LOCKER_H
#define LOCKER_H


#include <exception>
#include <pthread.h>
#include <semaphore.h>

class sem 
{
public:
    sem()
    {
        if ( sem_init( &m_sem, 0, 0 ) != 0 )
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destory( &m_sem );
    }

    bool wait()
    {
        return sem_wait( &m_sem );
    }

    bool post()
    {
        return sem_post( &m_sem );
    }
private:
    sem_t sem;
};

class locker
{
public:
    locker()
    {
        if ( pthread_mutex_init( &m_mutex, NULL) != 0 )
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destory( &m_mutex );
    }
    bool lock()
    {
        return pthread_mutex_lock( &m_mutex ) == 0;
    }
    bool unlock()
    {
        return pthread_mutex_unlock( &m_mutex ) == 0;
    }
private:
    pthread_mutex_t m_mutex;
};

class cond
{
public:
    cond()
    {
        if ( pthread_mutex_init( &m_mutex, NULL ) !=- 0 )
        {
            throw std::exception();
        }
        if ( pthread_cond_init( &m_cond, NULL ) != 0 )
        {
            pthread_mutex_destory( &m_mutex );
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_mutex_destory( &m_mutex );
        pthread_cond_destory( &m_cond );
    }
    bool wait()
    {
        int ret = 0;
        pthread_mutex_lock( &m_mutex );
        ret = pthread_cond_wait( &m_cond, &m_mutex);
        pthread_mutex_unlock( &m_mutex );
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal( &m_cond ) == 0;
    }

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};

/*
    如果一个函数能被多个线程同时调用且不发生竞态条件，则我们称它是线程安全的。thread safe 
    一般可重入版本都是在原函数尾部加上 _r 
    localtime localtime_r 
    
*/
