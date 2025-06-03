#ifndef OBC_H
#define OBC_H

#define IDLE 0
#define NOMINAL 1
#define LOW_POWER 2

#define LOW_POWER_THRESHOLD 10

#define NOMINAL_INTERVAL 20000 // 1000 = 1 second
#define LOW_POWER_INTERVAL 60000

void set_mode(int m);

#endif // OBC_H
