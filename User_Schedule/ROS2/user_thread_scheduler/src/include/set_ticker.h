#ifndef SET_TICKER_H
#define SET_TICKER_H

#include <unistd.h>
#include <sys/time.h>

/**
 * 设置定时器的触发间隔
 * @param n_msecs 触发间隔的时间，单位为毫秒
 * @return 成功返回0，失败返回非0值
 */
int set_ticker(int n_msecs);

/* 设置一个定时时长 */
int set_ticker(int n_msecs)
{
    struct itimerval new_timeset;
    long n_sec, n_usecs;//秒，微妙

    n_sec = n_msecs / 1000; //总秒数
    n_usecs = (n_msecs % 1000) * 1000L; //剩余的毫秒并转换成微妙

    //剩下的时间，tv.sec = 秒，tv.usec = 微秒
    new_timeset.it_interval.tv_sec = n_sec;
    new_timeset.it_interval.tv_usec = n_usecs;

    //间隔时间
    new_timeset.it_value.tv_sec = n_sec;
    new_timeset.it_value.tv_usec = n_usecs;

    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}

#endif
