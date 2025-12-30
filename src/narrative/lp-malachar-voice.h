/* lp-malachar-voice.h - Malachar Commentary System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * LpMalacharVoice provides sardonic commentary from Malachar the Undying.
 * Commentary is loaded from YAML files and varies based on context.
 * Supports multiple commentary variants per context for variety.
 */

#ifndef LP_MALACHAR_VOICE_H
#define LP_MALACHAR_VOICE_H

#include <glib-object.h>
#include <libregnum.h>
#include "../lp-types.h"
#include "../lp-enums.h"

G_BEGIN_DECLS

#define LP_TYPE_MALACHAR_VOICE (lp_malachar_voice_get_type ())

G_DECLARE_FINAL_TYPE (LpMalacharVoice, lp_malachar_voice, LP, MALACHAR_VOICE, GObject)

/* ==========================================================================
 * Singleton Access
 * ========================================================================== */

/**
 * lp_malachar_voice_get_default:
 *
 * Gets the default Malachar voice instance.
 * Initializes on first call, loading commentary from data files.
 *
 * Returns: (transfer none): The default #LpMalacharVoice
 */
LpMalacharVoice *
lp_malachar_voice_get_default (void);

/* ==========================================================================
 * Commentary Retrieval
 * ========================================================================== */

/**
 * lp_malachar_voice_get_commentary:
 * @self: an #LpMalacharVoice
 * @context: the commentary context
 *
 * Gets a random commentary for the given context.
 * Multiple commentaries may exist per context for variety.
 *
 * Returns: (transfer none): A commentary string
 */
const gchar *
lp_malachar_voice_get_commentary (LpMalacharVoice      *self,
                                   LpCommentaryContext   context);

/**
 * lp_malachar_voice_get_commentary_for_event:
 * @self: an #LpMalacharVoice
 * @event: the event to comment on
 *
 * Gets a random commentary appropriate for the given event.
 * Determines context from event type and properties.
 *
 * Returns: (transfer none): A commentary string
 */
const gchar *
lp_malachar_voice_get_commentary_for_event (LpMalacharVoice *self,
                                             LpEvent         *event);

/**
 * lp_malachar_voice_format_commentary:
 * @self: an #LpMalacharVoice
 * @context: the commentary context
 * @...: printf-style format arguments
 *
 * Gets a commentary and formats it with the given arguments.
 * The commentary string may contain format specifiers.
 *
 * Returns: (transfer full): A newly allocated formatted commentary
 */
gchar *
lp_malachar_voice_format_commentary (LpMalacharVoice      *self,
                                      LpCommentaryContext   context,
                                      ...);

/* ==========================================================================
 * Configuration
 * ========================================================================== */

/**
 * lp_malachar_voice_load_commentary:
 * @self: an #LpMalacharVoice
 * @path: path to the commentary YAML file
 * @error: return location for error, or %NULL
 *
 * Loads additional commentary from a YAML file.
 * Can be used for mods or DLC content.
 *
 * Returns: %TRUE on success
 */
gboolean
lp_malachar_voice_load_commentary (LpMalacharVoice  *self,
                                    const gchar      *path,
                                    GError          **error);

/**
 * lp_malachar_voice_get_commentary_count:
 * @self: an #LpMalacharVoice
 * @context: the commentary context
 *
 * Gets the number of commentary variants for a context.
 *
 * Returns: The number of commentaries
 */
guint
lp_malachar_voice_get_commentary_count (LpMalacharVoice      *self,
                                         LpCommentaryContext   context);

G_END_DECLS

#endif /* LP_MALACHAR_VOICE_H */
