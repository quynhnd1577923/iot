#ifndef INC_ERA_CONSOLE_H_
#define INC_ERA_CONSOLE_H_

#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <ERa/ERaParam.hpp>
#include <Utility/ERaUtility.hpp>

#define ERA_MAX_CMD 16

class ERaConsole
{
    typedef struct __EntryCmd_t {
        int pin;
        const char* cmd;
        float value;
    } EntryCmd_t;

    typedef enum __CmdStateT {
        CMD_STATE_BEGIN = 0,
        CMD_STATE_DATA = 1,
        CMD_STATE_END = 2
    } CmdStateT;

public:
    ERaConsole(Stream& st);
    ~ERaConsole();

    bool requestHumidity(unsigned long time);
    bool requestTemperature(unsigned long time);
    bool requestDistance(unsigned long time);
    bool waitResult(const char* cmd, unsigned long time);
    float getHumidity();
    float getTemperature();
    uint16_t getDistance();

    bool addCommand(int pin, const char* cmd);
    void init(int pinHumidity, int pinTemperature, int pinDistance);
    void begin();
    void run();
    void println();
    size_t write(const char* cmd);

private:
    void processInput();
    const char* processCommand();
    float getValue(const char* cmd);
    size_t splitString(char* strInput, const char* delims);

    Stream& stream;
    int numCommand;
    EntryCmd_t eCmd[ERA_MAX_CMD];
    unsigned long timeout;

    char buffer[1024];
    size_t bufferCount;
    CmdStateT cmdState;
};

#endif /* INC_ERA_CONSOLE_H_ */
