#include "SimonSays.h"
#include "DisplayManager.h"
#include "PeripheryManager.h"
#include "GameManager.h"

static const uint32_t colors[4] = {
    0x00FF00, // Green
    0xFF0000, // Red
    0xFFFF00, // Yellow
    0x0000FF  // Blue
};

SimonSays_ &SimonSays_::getInstance()
{
    static SimonSays_ instance;
    return instance;
}

SimonSays_ &SimonSays = SimonSays_::getInstance();

void SimonSays_::setup()
{
    generateSequence();
    currentState = SIMON_READY;
}

void SimonSays_::generateSequence()
{
    for (int i = 0; i < MAX_SEQ; i++)
    {
        sequence[i] = random(0, 4);
    }
    sequenceLength = 1;
    currentStep = 0;
}

void SimonSays_::tick()
{
    if (currentState == SIMON_SHOWSEQ)
    {
        showSequence();
    }

    if (highlightUserInput)
    {
        if (millis() >= highlightUserInputEnd)
        {
            highlightUserInput = false;
            showing = false;
            blinkIndex = -1;
        }
        else
        {
            blinkIndex = lastUserButton;
        }
    }
    drawSquares(showing, blinkIndex);

    if (currentState == SIMON_LOSE)
    {
        if (millis() - loseStart < 2000)
        {
            Serial.println("LOOSE");
            DisplayManager.printText(0, 0, "LOOSE", true, 0);
            return;
        }
        else
        {
            generateSequence();
            currentState = SIMON_SHOWSEQ;
        }
    }

    if (currentState == SIMON_PAUSE && millis() >= nextSequenceTime)
    {
        currentState = SIMON_SHOWSEQ;
        currentStep = 0;
    }
}

void SimonSays_::showSequence()
{
    unsigned long now = millis();
    if (now >= nextBlink)
    {
        showing = !showing;
        if (showing)
        {
            blinkIndex = sequence[currentStep];
            switch (blinkIndex)
            {
            case 0:
                PeripheryManager.playRTTTLString("seqGreen:d=8,o=5,b=120:c");
                break;
            case 1:
                PeripheryManager.playRTTTLString("seqRed:d=8,o=5,b=120:d");
                break;
            case 2:
                PeripheryManager.playRTTTLString("seqYellow:d=8,o=5,b=120:e");
                break;
            case 3:
                PeripheryManager.playRTTTLString("seqBlue:d=8,o=5,b=120:f");
                break;
            }
            nextBlink = now + 400;
        }
        else
        {
            blinkIndex = -1;
            currentStep++;
            nextBlink = now + 200;
            if (currentStep >= sequenceLength)
            {
                currentStep = 0;
                currentState = SIMON_USERINPUT;
            }
        }
    }
}

void SimonSays_::checkUserInput(int button)
{
    lastUserButton = button;
    highlightUserInput = true;
    highlightUserInputEnd = millis() + 200;
    if (sequence[currentStep] == button)
    {
        switch (button)
        {
        case 0:
            PeripheryManager.playRTTTLString("greenOk:d=8,o=5,b=120:c");
            break;
        case 1:
            PeripheryManager.playRTTTLString("redOk:d=8,o=5,b=120:d");
            break;
        case 2:
            PeripheryManager.playRTTTLString("yellowOk:d=8,o=5,b=120:e");
            break;
        case 3:
            PeripheryManager.playRTTTLString("blueOk:d=8,o=5,b=120:f");
            break;
        }
        currentStep++;
        if (currentStep >= sequenceLength)
        {
            sequenceLength++;
            if (sequenceLength > MAX_SEQ)
            {
                currentState = SIMON_WIN;
                // Bonus für das Durchspielen
                GameManager.sendPoints(1000);
            }
            else
            {
                // Punkte für jede erfolgreiche Sequenz
                GameManager.sendPoints(sequenceLength);
                nextSequenceTime = millis() + 1000;
                currentState = SIMON_PAUSE;
            }
        }
    }
    else
    {
        currentState = SIMON_LOSE;
        loseStart = millis();
        GameManager.sendPoints(0);
        PeripheryManager.playRTTTLString("loose:d=8,o=5,b=120:16c,16b,16a,4g");
    }
}

void SimonSays_::selectPressed()
{
    if (currentState == SIMON_READY || currentState == SIMON_LOSE || currentState == SIMON_WIN)
    {
        generateSequence();
        currentState = SIMON_SHOWSEQ;
    }
}

void SimonSays_::ControllerInput(const char *cmd)
{
    if (strcmp(cmd, "START") == 0 &&
        (currentState == SIMON_READY || currentState == SIMON_LOSE || currentState == SIMON_WIN))
    {
        generateSequence();
        currentState = SIMON_SHOWSEQ;
        return;
    }

    if (currentState == SIMON_USERINPUT)
    {
        if (strcmp(cmd, "ADOWN") == 0)
        {
            lastUserButton = 0;
            highlightUserInput = true;
            showing = true;
            highlightUserInputEnd = millis() + 200;
            checkUserInput(0);
        }
        else if (strcmp(cmd, "BDOWN") == 0)
        {
            lastUserButton = 1;
            highlightUserInput = true;
            showing = true;
            highlightUserInputEnd = millis() + 200;
            checkUserInput(1);
        }
        else if (strcmp(cmd, "CDOWN") == 0)
        {
            lastUserButton = 2;
            highlightUserInput = true;
            showing = true;
            highlightUserInputEnd = millis() + 200;
            checkUserInput(2);
        }
        else if (strcmp(cmd, "DDOWN") == 0)
        {
            lastUserButton = 3;
            highlightUserInput = true;
            showing = true;
            highlightUserInputEnd = millis() + 200;
            checkUserInput(3);
        }
    }
}

void SimonSays_::drawSquares(bool highlight, int highlightIndex)
{
    DisplayManager.drawFilledRect(0, 0, 32, 8, 0);
    DisplayManager.drawFilledRect(0, 0, 16, 4, (highlight && highlightIndex == 0) ? colors[0] : (colors[0] & 0x001500));
    DisplayManager.drawFilledRect(16, 0, 16, 4, (highlight && highlightIndex == 1) ? colors[1] : (colors[1] & 0x150000));
    DisplayManager.drawFilledRect(0, 4, 16, 4, (highlight && highlightIndex == 2) ? colors[2] : (colors[2] & 0x151500));
    DisplayManager.drawFilledRect(16, 4, 16, 4, (highlight && highlightIndex == 3) ? colors[3] : (colors[3] & 0x000015));
}
