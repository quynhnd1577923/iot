#define NO_GLOBAL_ERA

#include <math.h>
#include <ERaSimpleMBLinux.hpp>
#include <ERaConsole.h>

ERaConsole::ERaConsole(Stream& st)
    : stream(st)
    , numCommand(0)
    , timeout(1000UL)
    , bufferCount(0UL)
    , cmdState(CMD_STATE_BEGIN)
{}

ERaConsole::~ERaConsole()
{}

bool ERaConsole::requestHumidity(unsigned long time) {
    this->write("H");
    return this->waitResult("H", time);
}

bool ERaConsole::requestTemperature(unsigned long time) {
    this->write("T");
    return this->waitResult("T", time);
}

bool ERaConsole::requestDistance(unsigned long time) {
    this->write("S");
    return this->waitResult("S", time);
}

bool ERaConsole::waitResult(const char* cmdExpected, unsigned long time) {
    if (cmdExpected == NULL) {
        return false;
    }
    const char* cmd = "";
    const unsigned long startMillis = ERaMillis();
    do {
        processInput();
        if (this->cmdState == CMD_STATE_END) {
            cmd = this->processCommand();
            if (strcmp(cmd, cmdExpected) == 0) {
                return true;
            }
        }
    } while ((ERaMillis() - startMillis) < time);
    return false;
}

float ERaConsole::getHumidity() {
    return this->getValue("H");
}

float ERaConsole::getTemperature() {
    return this->getValue("T");
}

uint16_t ERaConsole::getDistance() {
    return this->getValue("S");
}

bool ERaConsole::addCommand(int pin, const char* cmd) {
    if (cmd == NULL) {
        return false;
    }
    if (this->numCommand >= ERA_MAX_CMD) {
        return false;
    }
    EntryCmd_t& e = this->eCmd[this->numCommand++];
    e.pin = pin;
    e.cmd = cmd;
    return true;
}

void ERaConsole::init(int pinHumidity, int pinTemperature, int pinDistance) {
    this->begin();
    this->addCommand(pinHumidity, "H");
    this->addCommand(pinTemperature, "T");
    this->addCommand(pinDistance, "S");
}

void ERaConsole::begin() {
    this->bufferCount = 0;
    this->cmdState = CMD_STATE_BEGIN;
    memset(this->buffer, 0, sizeof(this->buffer));
}

void ERaConsole::run() {
    processInput();
    if (this->cmdState == CMD_STATE_END) {
        this->processCommand();
    }
}

void ERaConsole::println() {
    this->stream.println();
}

size_t ERaConsole::write(const char* cmd) {
    if (cmd == NULL) {
        return 0;
    }
    return this->stream.write(cmd);
}

void ERaConsole::processInput() {
    if (!this->stream.available()) {
        return;
    }
    while (this->stream.available() && (this->bufferCount < sizeof(this->buffer))) {
        int c = this->stream.read();
        if (c < 0) {
            continue;
        }
        switch (this->cmdState) {
            case CMD_STATE_BEGIN:
                if (c == '!') {
                    this->cmdState = CMD_STATE_DATA;
                }
                break;
            case CMD_STATE_DATA:
                if (c == '#') {
                    this->cmdState = CMD_STATE_END;
                    return;
                }
                else {
                    this->buffer[this->bufferCount++] = (uint8_t)c;
                    this->bufferCount %= sizeof(this->buffer);
                }
                break;
            case CMD_STATE_END:
                if (c == '!') {
                    this->cmdState = CMD_STATE_DATA;
                }
                break;
        }
    }
}

const char* ERaConsole::processCommand() {
    if (!this->bufferCount) {
        return "";
    }
    ERaDataBuff data(this->buffer, this->bufferCount + 1, sizeof(this->buffer));
    this->buffer[this->bufferCount] = '\0';
    this->bufferCount = 0;
    this->splitString(this->buffer, ":");
    if (data.size() < 3) {
        return "";
    }
    if (data.at(0) != "1") {
        return "";
    }
    const char* cmd = data.at(1).getString();
    for (int i = 0; i < this->numCommand; ++i) {
        if (strcmp(this->eCmd[i].cmd, cmd) != 0) {
            continue;
        }
        this->eCmd[i].value = data.at(2).getDouble();
        ERa.virtualWrite(this->eCmd[i].pin, this->eCmd[i].value);
    }
    return cmd;
}

float ERaConsole::getValue(const char* cmd) {
    for (int i = 0; i < this->numCommand; ++i) {
        if (strcmp(this->eCmd[i].cmd, cmd) != 0) {
            continue;
        }
        return this->eCmd[i].value;
    }
    return 0;
}

size_t ERaConsole::splitString(char* strInput, const char* delims) {
    if ((strInput == NULL) ||
        (delims == NULL)) {
        return 0;
    }
    if (!strlen(strInput)) {
        return 0;
    }
    size_t size {0};
    char* token = strtok(strInput, delims);
    while (token != NULL) {
        ++size;
        token = strtok(NULL, delims);
    }
    return size;
}
