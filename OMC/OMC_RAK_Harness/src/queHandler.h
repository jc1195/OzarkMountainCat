#pragma once
#include "main.h"


class QueHandler
{
public:
    /**
     * @brief Default constructor.
     */
    QueHandler() {}

    /**
     * @brief Process any queued commands in a non-blocking manner.
     *
     * This function processes any queued commands in a non-blocking manner.
     * It receives the eventType from the commandQueue and processes the command accordingly.
     */
    void Que();
};