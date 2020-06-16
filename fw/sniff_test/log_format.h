#ifndef __LOG_FORMAT_H__
#define __LOG_FORMAT_H__

//-----------------------------------------------------------------
// Defines:
//-----------------------------------------------------------------

/*
   CTRL_TYPE   = top 4 bits (31:28)
 
   TYPE = LOG_CTRL_TYPE_SOF / RST
      CTRL_TYPE (31:28) | RST (11) | (FRAME # (10:0)
 
*/
// TYPE 1,2 (LOG_CTRL_TYPE_SOF / LOG_CTRL_TYPE_RST)
/* CTRL_TYPE (31:28) | RST (11) | (FRAME # (10:0)
 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|           |                                               |R |                                |
| CTRL_TYPE |   unused (16 bits)                            |S |           Frame # (11 bits)    |
|           |                                               |T |                                |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+ 
*/ 
#define LOG_SOF_FRAME_W         11
#define LOG_SOF_FRAME_MASK      ((1 << LOG_SOF_FRAME_W) - 1)
#define LOG_SOF_FRAME_L         0
#define LOG_SOF_FRAME_H         (LOG_SOF_FRAME_L + LOG_SOF_FRAME_W - 1)
#define LOG_RST_STATE_W         1
#define LOG_RST_STATE_MASK      ((1 << LOG_RST_STATE_W) - 1)
#define LOG_RST_STATE_L         (LOG_SOF_FRAME_H + 1)
#define LOG_RST_STATE_H         (LOG_RST_STATE_L + LOG_RST_STATE_W - 1)

// TYPE = LOG_CTRL_TYPE_TOKEN | LOG_CTRL_TYPE_HSHAKE | LOG_CTRL_TYPE_DATA
// CTRL_TYPE (31:28) | (PID # (3:0)
#define LOG_TOKEN_PID_W         4
#define LOG_TOKEN_PID_MASK      ((1 << LOG_TOKEN_PID_W) - 1)
#define LOG_TOKEN_PID_L         0
#define LOG_TOKEN_PID_H         (LOG_TOKEN_PID_L + LOG_TOKEN_PID_W - 1)

/* TYPE = LOG_CTRL_TYPE_TOKEN
// CTRL_TYPE (31:28) | TOKEN_DATA (19:4) | PID (3:0)

 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| CTRL_TYPE |  unused (8 bits)      |           Token data  (16 bits)               |    PID    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+ 
*/ 

#define LOG_TOKEN_DATA_W        16
#define LOG_TOKEN_DATA_MASK     ((1 << LOG_TOKEN_DATA_W) - 1)
#define LOG_TOKEN_DATA_L        (LOG_TOKEN_PID_H + 1)
#define LOG_TOKEN_DATA_H        (LOG_TOKEN_DATA_L + LOG_TOKEN_DATA_W - 1)

/* TYPE = LOG_CTRL_TYPE_DATA
 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| CTRL_TYPE |  unused (8 bits)      |            Len (16 bits)                      |    PID    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
#define LOG_DATA_LEN_W          16
#define LOG_DATA_LEN_MASK       ((1 << LOG_DATA_LEN_W) - 1)
#define LOG_DATA_LEN_L          (LOG_TOKEN_PID_H + 1)
#define LOG_DATA_LEN_H          (LOG_DATA_LEN_L + LOG_DATA_LEN_W - 1)

/* TYPE = LOG_CTRL_TYPE_TOKEN | LOG_CTRL_TYPE_HSHAKE | LOG_CTRL_TYPE_DATA
 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
| CTRL_TYPE |  cycle (8 bits)       |            Len (16 bits)                      |    PID    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
*/
#define LOG_CTRL_CYCLE_W         8
#define LOG_CTRL_CYCLE_MASK      ((1 << LOG_CTRL_CYCLE_W) - 1)
#define LOG_CTRL_CYCLE_L         20
#define LOG_CTRL_CYCLE_H         (LOG_CTRL_CYCLE_L + LOG_CTRL_CYCLE_W - 1)

#define LOG_CTRL_TYPE_W          4
#define LOG_CTRL_TYPE_L          28
#define LOG_CTRL_TYPE_H          31
#define LOG_CTRL_TYPE_SOF        0x1
#define LOG_CTRL_TYPE_RST        0x2
#define LOG_CTRL_TYPE_TOKEN      0x3
#define LOG_CTRL_TYPE_HSHAKE     0x4
#define LOG_CTRL_TYPE_DATA       0x5

#endif
