/* main.c - Entry Point for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Main entry point for the game. Creates the application singleton
 * and runs the main game loop.
 */

#include <glib.h>
#include "core/lp-application.h"

int
main (int    argc,
      char **argv)
{
    LpApplication *app;
    gint           status;

    app = lp_application_get_default ();
    status = lp_application_run (app, argc, argv);
    g_object_unref (app);

    return status;
}
