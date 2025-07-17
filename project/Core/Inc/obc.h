#ifndef OBC_H
#define OBC_H

#define NOMINAL_MODE 0
#define LOW_POWER_MODE 1

#define LOW_POWER_THRESHOLD 10 // TODO: Replace with actual value

// 1000 = 1 second
#define NOMINAL_INTERVAL 5000 // 5 seconds
#define LOW_POWER_INTERVAL 20000 // 20 seconds
#define IMAGE_INTERVAL 30000 // 30 seconds

void set_mode(int m);

#endif // OBC_H
