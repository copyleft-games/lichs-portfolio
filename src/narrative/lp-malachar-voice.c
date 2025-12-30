/* lp-malachar-voice.c - Malachar Commentary System
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#define LP_LOG_DOMAIN LP_LOG_DOMAIN_CORE
#include "../lp-log.h"

#include "lp-malachar-voice.h"
#include "../simulation/lp-event.h"

/* Number of commentary context types */
#define COMMENTARY_CONTEXT_COUNT 14

struct _LpMalacharVoice
{
    GObject       parent_instance;

    /* Commentary storage: array of GPtrArray per context */
    GPtrArray    *commentaries[COMMENTARY_CONTEXT_COUNT];

    /* Data directory */
    gchar        *data_dir;

    /* Random number generator for variety */
    GRand        *rand;
};

G_DEFINE_FINAL_TYPE (LpMalacharVoice, lp_malachar_voice, G_TYPE_OBJECT)

static LpMalacharVoice *default_voice = NULL;

/* ==========================================================================
 * Default Commentary Fallbacks
 * ========================================================================== */

static const gchar *default_commentaries[] = {
    /* LP_COMMENTARY_GREETING */
    "Ah, you've awakened. Much has transpired...",
    /* LP_COMMENTARY_SLUMBER */
    "Time to rest. The world continues without us.",
    /* LP_COMMENTARY_KINGDOM_COLLAPSE */
    "Another kingdom crumbles. Their vaults remain.",
    /* LP_COMMENTARY_AGENT_DEATH */
    "Mortals and their brief candles...",
    /* LP_COMMENTARY_AGENT_BETRAYAL */
    "Treachery. How disappointingly predictable.",
    /* LP_COMMENTARY_COMPETITOR_DEFEAT */
    "One fewer rival. The centuries grow lonely.",
    /* LP_COMMENTARY_DISCOVERY */
    "They search for my phylactery. How adorable.",
    /* LP_COMMENTARY_CRUSADE */
    "They rally their armies against shadows.",
    /* LP_COMMENTARY_INVESTMENT_SUCCESS */
    "Gold begets gold. Always.",
    /* LP_COMMENTARY_INVESTMENT_FAILURE */
    "Even immortals make mistakes. We have time.",
    /* LP_COMMENTARY_PRESTIGE */
    "Another cycle complete. We grow stronger.",
    /* LP_COMMENTARY_FIRST_INVESTMENT */
    "Your first investment. The foundation of empire.",
    /* LP_COMMENTARY_FIRST_AGENT */
    "Your first servant. Many more will follow.",
    /* LP_COMMENTARY_FIRST_SLUMBER */
    "Time to experience the patience of eternity."
};

/* ==========================================================================
 * Private Implementation
 * ========================================================================== */

static void
lp_malachar_voice_ensure_defaults (LpMalacharVoice *self)
{
    guint i;

    /*
     * Ensure each context has at least the default commentary.
     * This is called during initialization and provides fallbacks.
     */
    for (i = 0; i < COMMENTARY_CONTEXT_COUNT; i++)
    {
        if (self->commentaries[i]->len == 0)
        {
            g_ptr_array_add (self->commentaries[i],
                             g_strdup (default_commentaries[i]));
        }
    }
}

static gboolean
lp_malachar_voice_parse_yaml (LpMalacharVoice  *self,
                               const gchar      *path,
                               GError          **error)
{
    g_autoptr(GKeyFile) keyfile = NULL;
    gchar **contexts = NULL;
    gsize n_contexts = 0;
    gsize i;

    /*
     * Load commentary from YAML file.
     * Format:
     * greeting:
     *   - "Commentary 1"
     *   - "Commentary 2"
     * kingdom-collapse:
     *   - "Commentary 1"
     */

    /* For simplicity, use GKeyFile-compatible format or parse manually */
    /* TODO: Use yaml-glib for proper YAML parsing */

    lp_log_debug ("Loading commentary from: %s", path);

    if (!g_file_test (path, G_FILE_TEST_EXISTS))
    {
        lp_log_debug ("Commentary file not found, using defaults: %s", path);
        return TRUE;  /* Not an error, just use defaults */
    }

    /* Parse file as simple key=value pairs for now */
    keyfile = g_key_file_new ();
    if (!g_key_file_load_from_file (keyfile, path,
                                     G_KEY_FILE_NONE, error))
    {
        return FALSE;
    }

    contexts = g_key_file_get_groups (keyfile, &n_contexts);

    for (i = 0; i < n_contexts; i++)
    {
        const gchar *context_name = contexts[i];
        gchar **lines = NULL;
        gsize n_lines = 0;
        LpCommentaryContext ctx;
        gsize j;

        /* Map context name to enum */
        if (g_str_equal (context_name, "greeting"))
            ctx = LP_COMMENTARY_GREETING;
        else if (g_str_equal (context_name, "slumber"))
            ctx = LP_COMMENTARY_SLUMBER;
        else if (g_str_equal (context_name, "kingdom-collapse"))
            ctx = LP_COMMENTARY_KINGDOM_COLLAPSE;
        else if (g_str_equal (context_name, "agent-death"))
            ctx = LP_COMMENTARY_AGENT_DEATH;
        else if (g_str_equal (context_name, "agent-betrayal"))
            ctx = LP_COMMENTARY_AGENT_BETRAYAL;
        else if (g_str_equal (context_name, "competitor-defeat"))
            ctx = LP_COMMENTARY_COMPETITOR_DEFEAT;
        else if (g_str_equal (context_name, "discovery"))
            ctx = LP_COMMENTARY_DISCOVERY;
        else if (g_str_equal (context_name, "crusade"))
            ctx = LP_COMMENTARY_CRUSADE;
        else if (g_str_equal (context_name, "investment-success"))
            ctx = LP_COMMENTARY_INVESTMENT_SUCCESS;
        else if (g_str_equal (context_name, "investment-failure"))
            ctx = LP_COMMENTARY_INVESTMENT_FAILURE;
        else if (g_str_equal (context_name, "prestige"))
            ctx = LP_COMMENTARY_PRESTIGE;
        else if (g_str_equal (context_name, "first-investment"))
            ctx = LP_COMMENTARY_FIRST_INVESTMENT;
        else if (g_str_equal (context_name, "first-agent"))
            ctx = LP_COMMENTARY_FIRST_AGENT;
        else if (g_str_equal (context_name, "first-slumber"))
            ctx = LP_COMMENTARY_FIRST_SLUMBER;
        else
        {
            lp_log_warning ("Unknown commentary context: %s", context_name);
            continue;
        }

        /* Load lines for this context */
        lines = g_key_file_get_string_list (keyfile, context_name,
                                            "lines", &n_lines, NULL);
        if (lines != NULL)
        {
            for (j = 0; j < n_lines; j++)
            {
                g_ptr_array_add (self->commentaries[ctx],
                                 g_strdup (lines[j]));
            }
            g_strfreev (lines);
        }
    }

    g_strfreev (contexts);
    return TRUE;
}

/* ==========================================================================
 * GObject Implementation
 * ========================================================================== */

static void
lp_malachar_voice_finalize (GObject *object)
{
    LpMalacharVoice *self = LP_MALACHAR_VOICE (object);
    guint i;

    for (i = 0; i < COMMENTARY_CONTEXT_COUNT; i++)
    {
        g_ptr_array_unref (self->commentaries[i]);
    }

    g_clear_pointer (&self->data_dir, g_free);
    g_clear_pointer (&self->rand, g_rand_free);

    G_OBJECT_CLASS (lp_malachar_voice_parent_class)->finalize (object);
}

static void
lp_malachar_voice_class_init (LpMalacharVoiceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = lp_malachar_voice_finalize;
}

static void
lp_malachar_voice_init (LpMalacharVoice *self)
{
    guint i;

    /* Initialize commentary arrays */
    for (i = 0; i < COMMENTARY_CONTEXT_COUNT; i++)
    {
        self->commentaries[i] = g_ptr_array_new_with_free_func (g_free);
    }

    /* Initialize random generator */
    self->rand = g_rand_new ();

    /* Determine data directory */
    if (g_file_test ("data/narrative", G_FILE_TEST_IS_DIR))
    {
        self->data_dir = g_strdup ("data");
    }
    else
    {
        self->data_dir = g_build_filename (
            g_get_user_data_dir (), "lichs-portfolio", NULL);
    }
}

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
lp_malachar_voice_get_default (void)
{
    if (default_voice == NULL)
    {
        g_autoptr(GError) error = NULL;
        g_autofree gchar *path = NULL;

        default_voice = g_object_new (LP_TYPE_MALACHAR_VOICE, NULL);

        /* Load commentary from file */
        path = g_build_filename (default_voice->data_dir,
                                  "narrative", "commentary.yaml", NULL);

        if (!lp_malachar_voice_parse_yaml (default_voice, path, &error))
        {
            lp_log_warning ("Failed to load commentary: %s", error->message);
        }

        /* Ensure defaults are available */
        lp_malachar_voice_ensure_defaults (default_voice);
    }

    return default_voice;
}

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
                                   LpCommentaryContext   context)
{
    GPtrArray *comments;
    guint index;

    g_return_val_if_fail (LP_IS_MALACHAR_VOICE (self), "...");
    g_return_val_if_fail (context < COMMENTARY_CONTEXT_COUNT,
                          default_commentaries[0]);

    comments = self->commentaries[context];

    if (comments->len == 0)
    {
        return default_commentaries[context];
    }

    /* Pick a random commentary from the available options */
    index = g_rand_int_range (self->rand, 0, comments->len);
    return g_ptr_array_index (comments, index);
}

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
                                             LpEvent         *event)
{
    LpEventType event_type;
    LpCommentaryContext context;

    g_return_val_if_fail (LP_IS_MALACHAR_VOICE (self), "...");
    g_return_val_if_fail (LP_IS_EVENT (event), "...");

    event_type = lp_event_get_event_type (event);

    /*
     * Map event type to commentary context.
     * This is a simplified mapping - could be more sophisticated.
     */
    switch (event_type)
    {
        case LP_EVENT_TYPE_ECONOMIC:
            /* Check if positive or negative by event subtype */
            context = LP_COMMENTARY_INVESTMENT_SUCCESS;
            break;

        case LP_EVENT_TYPE_POLITICAL:
            context = LP_COMMENTARY_KINGDOM_COLLAPSE;
            break;

        case LP_EVENT_TYPE_MAGICAL:
            context = LP_COMMENTARY_DISCOVERY;
            break;

        case LP_EVENT_TYPE_PERSONAL:
            context = LP_COMMENTARY_AGENT_DEATH;
            break;

        default:
            context = LP_COMMENTARY_GREETING;
            break;
    }

    return lp_malachar_voice_get_commentary (self, context);
}

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
                                      ...)
{
    const gchar *format;
    gchar *result;
    va_list args;

    g_return_val_if_fail (LP_IS_MALACHAR_VOICE (self), NULL);

    format = lp_malachar_voice_get_commentary (self, context);

    va_start (args, context);
    result = g_strdup_vprintf (format, args);
    va_end (args);

    return result;
}

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
                                    GError          **error)
{
    g_return_val_if_fail (LP_IS_MALACHAR_VOICE (self), FALSE);
    g_return_val_if_fail (path != NULL, FALSE);

    return lp_malachar_voice_parse_yaml (self, path, error);
}

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
                                         LpCommentaryContext   context)
{
    g_return_val_if_fail (LP_IS_MALACHAR_VOICE (self), 0);
    g_return_val_if_fail (context < COMMENTARY_CONTEXT_COUNT, 0);

    return self->commentaries[context]->len;
}
