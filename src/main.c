/* main.c - Entry Point for Lich's Portfolio
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Main entry point for the game. This is a Phase 0 stub that will be
 * expanded in Phase 1 when LpApplication is implemented.
 */

#include <glib.h>
#include <stdio.h>

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_APP
#include "lp-log.h"
#include "lp-enums.h"

/*
 * Phase 0 Stub
 *
 * This is a placeholder until Phase 1 implements LpApplication.
 * The final main() will look like:
 *
 *   #include "core/lp-application.h"
 *
 *   int
 *   main (int    argc,
 *         char **argv)
 *   {
 *       LpApplication *app;
 *       int status;
 *
 *       app = lp_application_get_default ();
 *       status = lp_application_run (app, argc, argv);
 *       g_object_unref (app);
 *
 *       return status;
 *   }
 */

int
main (int    argc,
      char **argv)
{
    g_print ("Lich's Portfolio - Phase 0 Stub\n");
    g_print ("Build system initialized successfully.\n");
    g_print ("\n");
    g_print ("GLib version: %d.%d.%d\n",
             glib_major_version,
             glib_minor_version,
             glib_micro_version);

    /* Verify enum registration works */
    g_print ("\n");
    g_print ("Enum types registered:\n");
    g_print ("  - LpAssetClass:     %s\n", g_type_name (LP_TYPE_ASSET_CLASS));
    g_print ("  - LpAgentType:      %s\n", g_type_name (LP_TYPE_AGENT_TYPE));
    g_print ("  - LpRiskLevel:      %s\n", g_type_name (LP_TYPE_RISK_LEVEL));
    g_print ("  - LpEventType:      %s\n", g_type_name (LP_TYPE_EVENT_TYPE));
    g_print ("  - LpExposureLevel:  %s\n", g_type_name (LP_TYPE_EXPOSURE_LEVEL));
    g_print ("  - LpLedgerCategory: %s\n", g_type_name (LP_TYPE_LEDGER_CATEGORY));
    g_print ("  - LpCoverStatus:    %s\n", g_type_name (LP_TYPE_COVER_STATUS));
    g_print ("  - LpKnowledgeLevel: %s\n", g_type_name (LP_TYPE_KNOWLEDGE_LEVEL));

    g_print ("\n");
    g_print ("Phase 0 complete. Implement Phase 1 (LpApplication) to continue.\n");

    (void)argc;
    (void)argv;

    return 0;
}
