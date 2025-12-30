/* test-feedback.c - Phase 6.5 Feedback System Tests
 *
 * Copyright 2025 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for feedback systems: floating text, growth particles,
 * synergy effects, achievement popups, and slumber visualization.
 */

#include <glib.h>
#include <libregnum.h>

#include "feedback/lp-floating-text.h"
#include "feedback/lp-growth-particles.h"
#include "feedback/lp-synergy-effect.h"
#include "feedback/lp-achievement-popup.h"
#include "feedback/lp-slumber-visualization.h"
#include "ui/lp-theme.h"
#include "lp-enums.h"

/* ==========================================================================
 * Floating Text Tests
 * ========================================================================== */

static void
test_floating_text_new (void)
{
    g_autoptr(GrlColor) color = grl_color_new (255, 200, 100, 255);
    g_autoptr(LpFloatingText) text = NULL;

    text = lp_floating_text_new ("Test", 100.0f, 200.0f, color);
    g_assert_nonnull (text);
    g_assert_true (LP_IS_FLOATING_TEXT (text));
}

static void
test_floating_text_properties (void)
{
    g_autoptr(GrlColor) color = grl_color_new (255, 200, 100, 255);
    g_autoptr(LpFloatingText) text = NULL;
    const gchar *text_str;

    text = lp_floating_text_new ("+1000 gp", 100.0f, 200.0f, color);

    /* Test text property */
    text_str = lp_floating_text_get_text (text);
    g_assert_cmpstr (text_str, ==, "+1000 gp");

    lp_floating_text_set_text (text, "Different");
    text_str = lp_floating_text_get_text (text);
    g_assert_cmpstr (text_str, ==, "Different");
}

static void
test_floating_text_lifetime (void)
{
    g_autoptr(GrlColor) color = grl_color_new (255, 200, 100, 255);
    g_autoptr(LpFloatingText) text = NULL;

    text = lp_floating_text_new ("Test", 0.0f, 0.0f, color);

    /* Test default lifetime */
    g_assert_cmpfloat (lp_floating_text_get_lifetime (text), ==, 2.0f);

    /* Test setting lifetime */
    lp_floating_text_set_lifetime (text, 5.0f);
    g_assert_cmpfloat (lp_floating_text_get_lifetime (text), ==, 5.0f);
}

static void
test_floating_text_velocity (void)
{
    g_autoptr(GrlColor) color = grl_color_new (255, 200, 100, 255);
    g_autoptr(LpFloatingText) text = NULL;

    text = lp_floating_text_new ("Test", 0.0f, 0.0f, color);

    /* Test default velocity (negative = upward) */
    g_assert_cmpfloat (lp_floating_text_get_velocity_y (text), ==, -50.0f);

    /* Test setting velocity */
    lp_floating_text_set_velocity_y (text, -100.0f);
    g_assert_cmpfloat (lp_floating_text_get_velocity_y (text), ==, -100.0f);
}

static void
test_floating_text_animation (void)
{
    g_autoptr(GrlColor) color = grl_color_new (255, 200, 100, 255);
    g_autoptr(LpFloatingText) text = NULL;

    text = lp_floating_text_new ("Test", 0.0f, 0.0f, color);
    lp_floating_text_set_lifetime (text, 1.0f);

    /* Initially not finished */
    g_assert_false (lp_floating_text_is_finished (text));

    /* Alpha starts at 1.0 */
    g_assert_cmpfloat (lp_floating_text_get_alpha (text), ==, 1.0f);

    /* Update partway through */
    lp_floating_text_update (text, 0.5f);
    g_assert_false (lp_floating_text_is_finished (text));
    g_assert_cmpfloat (lp_floating_text_get_alpha (text), ==, 1.0f);  /* No fade yet */

    /* Update past 50% - should start fading */
    lp_floating_text_update (text, 0.3f);
    g_assert_false (lp_floating_text_is_finished (text));
    g_assert_cmpfloat (lp_floating_text_get_alpha (text), <, 1.0f);

    /* Update to finish */
    lp_floating_text_update (text, 0.5f);
    g_assert_true (lp_floating_text_is_finished (text));
}

/* ==========================================================================
 * Growth Particles Tests
 * ========================================================================== */

static void
test_growth_particles_new (void)
{
    g_autoptr(LpGrowthParticles) particles = NULL;

    particles = lp_growth_particles_new ();
    g_assert_nonnull (particles);
    g_assert_true (LP_IS_GROWTH_PARTICLES (particles));
}

static void
test_growth_particles_intensity (void)
{
    g_autoptr(LpGrowthParticles) particles = NULL;

    particles = lp_growth_particles_new ();

    /* Test initial intensity */
    g_assert_cmpint (lp_growth_particles_get_intensity (particles), ==,
                     LP_GROWTH_INTENSITY_MINOR);

    /* Test spawning at different intensities */
    lp_growth_particles_spawn (particles, 100.0f, 100.0f, LP_GROWTH_INTENSITY_MODERATE);
    g_assert_cmpint (lp_growth_particles_get_intensity (particles), ==,
                     LP_GROWTH_INTENSITY_MODERATE);

    lp_growth_particles_spawn (particles, 100.0f, 100.0f, LP_GROWTH_INTENSITY_MAJOR);
    g_assert_cmpint (lp_growth_particles_get_intensity (particles), ==,
                     LP_GROWTH_INTENSITY_MAJOR);

    lp_growth_particles_spawn (particles, 100.0f, 100.0f, LP_GROWTH_INTENSITY_LEGENDARY);
    g_assert_cmpint (lp_growth_particles_get_intensity (particles), ==,
                     LP_GROWTH_INTENSITY_LEGENDARY);
}

static void
test_growth_particles_lifecycle (void)
{
    g_autoptr(LpGrowthParticles) particles = NULL;

    particles = lp_growth_particles_new ();

    /* Initially not alive (no particles spawned) */
    g_assert_false (lp_growth_particles_is_alive (particles));

    /* Spawn particles */
    lp_growth_particles_spawn (particles, 100.0f, 100.0f, LP_GROWTH_INTENSITY_MINOR);
    g_assert_true (lp_growth_particles_is_alive (particles));

    /* Clear particles */
    lp_growth_particles_clear (particles);
    g_assert_false (lp_growth_particles_is_alive (particles));
}

/* ==========================================================================
 * Synergy Effect Tests
 * ========================================================================== */

static void
test_synergy_effect_new (void)
{
    g_autoptr(LpSynergyEffect) effect = NULL;

    effect = lp_synergy_effect_new ();
    g_assert_nonnull (effect);
    g_assert_true (LP_IS_SYNERGY_EFFECT (effect));
}

static void
test_synergy_effect_endpoints (void)
{
    g_autoptr(LpSynergyEffect) effect = NULL;

    effect = lp_synergy_effect_new ();

    /* Set endpoints */
    lp_synergy_effect_set_endpoints (effect, 10.0f, 20.0f, 100.0f, 200.0f);

    /* Verify not complete yet */
    g_assert_false (lp_synergy_effect_is_complete (effect));
}

static void
test_synergy_effect_animation (void)
{
    g_autoptr(LpSynergyEffect) effect = NULL;

    effect = lp_synergy_effect_new ();

    /* Initial progress is 0 */
    g_assert_cmpfloat (lp_synergy_effect_get_progress (effect), ==, 0.0f);
    g_assert_false (lp_synergy_effect_is_complete (effect));

    /* Update partway */
    lp_synergy_effect_update (effect, 0.5f);
    g_assert_cmpfloat (lp_synergy_effect_get_progress (effect), >, 0.0f);
    g_assert_cmpfloat (lp_synergy_effect_get_progress (effect), <, 1.0f);
    g_assert_false (lp_synergy_effect_is_complete (effect));

    /* Update to completion (default duration is 1.0s) */
    lp_synergy_effect_update (effect, 1.0f);
    g_assert_cmpfloat (lp_synergy_effect_get_progress (effect), ==, 1.0f);
    g_assert_true (lp_synergy_effect_is_complete (effect));
}

/* ==========================================================================
 * Achievement Popup Tests
 * ========================================================================== */

static void
test_achievement_popup_new (void)
{
    g_autoptr(LpAchievementPopup) popup = NULL;

    popup = lp_achievement_popup_new ();
    g_assert_nonnull (popup);
    g_assert_true (LP_IS_ACHIEVEMENT_POPUP (popup));
}

static void
test_achievement_popup_visibility (void)
{
    g_autoptr(LpAchievementPopup) popup = NULL;

    popup = lp_achievement_popup_new ();

    /* Initially hidden */
    g_assert_false (lp_achievement_popup_is_visible (popup));

    /* Show popup */
    lp_achievement_popup_show (popup, "First Million", "Reach 1,000,000 gp");
    g_assert_true (lp_achievement_popup_is_visible (popup));

    /* Verify content */
    g_assert_cmpstr (lp_achievement_popup_get_name (popup), ==, "First Million");
    g_assert_cmpstr (lp_achievement_popup_get_description (popup), ==, "Reach 1,000,000 gp");

    /* Dismiss popup */
    lp_achievement_popup_dismiss (popup);
    /* Note: Still visible during slide-out animation */
}

static void
test_achievement_popup_auto_dismiss (void)
{
    g_autoptr(LpAchievementPopup) popup = NULL;

    popup = lp_achievement_popup_new ();

    /* Test default auto-dismiss time */
    g_assert_cmpfloat (lp_achievement_popup_get_auto_dismiss_time (popup), ==, 5.0f);

    /* Test setting auto-dismiss time */
    lp_achievement_popup_set_auto_dismiss_time (popup, 3.0f);
    g_assert_cmpfloat (lp_achievement_popup_get_auto_dismiss_time (popup), ==, 3.0f);

    /* Test disabling auto-dismiss */
    lp_achievement_popup_set_auto_dismiss_time (popup, 0.0f);
    g_assert_cmpfloat (lp_achievement_popup_get_auto_dismiss_time (popup), ==, 0.0f);
}

/* ==========================================================================
 * Slumber Visualization Tests
 * ========================================================================== */

static void
test_slumber_visualization_new (void)
{
    g_autoptr(LpSlumberVisualization) viz = NULL;

    viz = lp_slumber_visualization_new ();
    g_assert_nonnull (viz);
    g_assert_true (LP_IS_SLUMBER_VISUALIZATION (viz));
}

static void
test_slumber_visualization_lifecycle (void)
{
    g_autoptr(LpSlumberVisualization) viz = NULL;

    viz = lp_slumber_visualization_new ();

    /* Initially not active */
    g_assert_false (lp_slumber_visualization_is_active (viz));

    /* Start visualization */
    lp_slumber_visualization_start (viz, 847, 947);
    g_assert_true (lp_slumber_visualization_is_active (viz));
    g_assert_cmpuint (lp_slumber_visualization_get_current_year (viz), ==, 847);
    g_assert_cmpuint (lp_slumber_visualization_get_target_year (viz), ==, 947);

    /* Stop visualization */
    lp_slumber_visualization_stop (viz);
    g_assert_false (lp_slumber_visualization_is_active (viz));
}

static void
test_slumber_visualization_year (void)
{
    g_autoptr(LpSlumberVisualization) viz = NULL;

    viz = lp_slumber_visualization_new ();
    lp_slumber_visualization_start (viz, 847, 947);

    /* Update year */
    lp_slumber_visualization_set_year (viz, 900);
    g_assert_cmpuint (lp_slumber_visualization_get_current_year (viz), ==, 900);

    lp_slumber_visualization_set_year (viz, 947);
    g_assert_cmpuint (lp_slumber_visualization_get_current_year (viz), ==, 947);
}

static void
test_slumber_visualization_acceleration (void)
{
    g_autoptr(LpSlumberVisualization) viz = NULL;

    viz = lp_slumber_visualization_new ();
    lp_slumber_visualization_start (viz, 847, 947);

    /* Initially not accelerating */
    g_assert_false (lp_slumber_visualization_is_accelerating (viz));
    g_assert_cmpfloat (lp_slumber_visualization_get_simulation_speed (viz), ==, 1.0f);

    /* Enable acceleration */
    lp_slumber_visualization_accelerate (viz, TRUE);
    g_assert_true (lp_slumber_visualization_is_accelerating (viz));
    g_assert_cmpfloat (lp_slumber_visualization_get_simulation_speed (viz), ==, 5.0f);

    /* Disable acceleration */
    lp_slumber_visualization_accelerate (viz, FALSE);
    g_assert_false (lp_slumber_visualization_is_accelerating (viz));
    g_assert_cmpfloat (lp_slumber_visualization_get_simulation_speed (viz), ==, 1.0f);
}

static void
test_slumber_visualization_events (void)
{
    g_autoptr(LpSlumberVisualization) viz = NULL;

    viz = lp_slumber_visualization_new ();
    lp_slumber_visualization_start (viz, 847, 947);

    /* Clear events should work (no events to clear) */
    lp_slumber_visualization_clear_events (viz);

    /* Note: Cannot easily test add_event without creating LpEvent objects,
     * but we can verify the clear function works */
}

/* ==========================================================================
 * Main Entry Point
 * ========================================================================== */

int
main (int    argc,
      char **argv)
{
    g_test_init (&argc, &argv, NULL);

    /* Initialize theme for all tests */
    lp_theme_configure_default ();

    /* Floating text tests */
    g_test_add_func ("/feedback/floating-text/new", test_floating_text_new);
    g_test_add_func ("/feedback/floating-text/properties", test_floating_text_properties);
    g_test_add_func ("/feedback/floating-text/lifetime", test_floating_text_lifetime);
    g_test_add_func ("/feedback/floating-text/velocity", test_floating_text_velocity);
    g_test_add_func ("/feedback/floating-text/animation", test_floating_text_animation);

    /* Growth particles tests */
    g_test_add_func ("/feedback/growth-particles/new", test_growth_particles_new);
    g_test_add_func ("/feedback/growth-particles/intensity", test_growth_particles_intensity);
    g_test_add_func ("/feedback/growth-particles/lifecycle", test_growth_particles_lifecycle);

    /* Synergy effect tests */
    g_test_add_func ("/feedback/synergy-effect/new", test_synergy_effect_new);
    g_test_add_func ("/feedback/synergy-effect/endpoints", test_synergy_effect_endpoints);
    g_test_add_func ("/feedback/synergy-effect/animation", test_synergy_effect_animation);

    /* Achievement popup tests */
    g_test_add_func ("/feedback/achievement-popup/new", test_achievement_popup_new);
    g_test_add_func ("/feedback/achievement-popup/visibility", test_achievement_popup_visibility);
    g_test_add_func ("/feedback/achievement-popup/auto-dismiss", test_achievement_popup_auto_dismiss);

    /* Slumber visualization tests */
    g_test_add_func ("/feedback/slumber-visualization/new", test_slumber_visualization_new);
    g_test_add_func ("/feedback/slumber-visualization/lifecycle", test_slumber_visualization_lifecycle);
    g_test_add_func ("/feedback/slumber-visualization/year", test_slumber_visualization_year);
    g_test_add_func ("/feedback/slumber-visualization/acceleration", test_slumber_visualization_acceleration);
    g_test_add_func ("/feedback/slumber-visualization/events", test_slumber_visualization_events);

    return g_test_run ();
}
