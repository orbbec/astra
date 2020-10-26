// This file is part of the Orbbec Astra SDK [https://orbbec3d.com]
// Copyright (c) 2015 Orbbec 3D
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Be excellent to each other.
#ifndef KEY_HANDLER_H
#define KEY_HANDLER_H

#include <stdio.h>
#include <stdbool.h>

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static bool shouldContinue = true;

void signal_stop_processing(){
    printf("quitting...\n\n");
    shouldContinue = false;
}

static void signal_handler(int s) {
    signal_stop_processing();
}

static void set_key_handler()
{
  struct sigaction sigIntHandler;

  sigIntHandler.sa_handler = &signal_handler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;

  sigaction(SIGINT, &sigIntHandler, NULL);

}

#endif /* KEY_HANDLER_H */
