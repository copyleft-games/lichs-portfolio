/* lp-accessibility-panel.c - Accessibility settings panel widget
 *
 * Copyright 2026 Zach Podbielniak
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "lp-accessibility-panel.h"

struct _LpAccessibilityPanel
{
    LrgWidget                 parent_instance;

    LrgAccessibilitySettings *settings;

    /* Visual settings */
    LrgWidget *colorblind_label;
    LrgWidget *colorblind_selector;
    LrgWidget *high_contrast_label;
    LrgWidget *high_contrast_toggle;
    LrgWidget *ui_scale_label;
    LrgWidget *ui_scale_slider;
    LrgWidget *reduce_motion_label;
    LrgWidget *reduce_motion_toggle;

    /* Selected values */
    gint       colorblind_index;
    gboolean   high_contrast;
    gfloat     ui_scale;
    gboolean   reduce_motion;
};

G_DEFINE_TYPE (LpAccessibilityPanel, lp_accessibility_panel, LRG_TYPE_WIDGET)

static const gchar *COLORBLIND_OPTIONS[] = {
    "None",
    "Deuteranopia (Red-Green)",
    "Protanopia (Red-Green)",
    "Tritanopia (Blue-Yellow)",
    "Achromatopsia (Grayscale)",
    NULL
};

static void
on_colorblind_changed (LrgWidget *selector,
                       gpointer   user_data)
{
    LpAccessibilityPanel *self = LP_ACCESSIBILITY_PANEL (user_data);

    self->colorblind_index = lrg_selector_get_selected_index (LRG_SELECTOR (selector));
    lrg_accessibility_settings_set_colorblind_type (self->settings,
                                                    (LrgColorblindType)self->colorblind_index);
}

static void
on_high_contrast_toggled (LrgWidget *toggle,
                          gpointer   user_data)
{
    LpAccessibilityPanel *self = LP_ACCESSIBILITY_PANEL (user_data);

    self->high_contrast = lrg_toggle_get_active (LRG_TOGGLE (toggle));
    lrg_accessibility_settings_set_high_contrast (self->settings, self->high_contrast);
}

static void
on_ui_scale_changed (LrgWidget *slider,
                     gpointer   user_data)
{
    LpAccessibilityPanel *self = LP_ACCESSIBILITY_PANEL (user_data);

    self->ui_scale = lrg_slider_get_value (LRG_SLIDER (slider));
    lrg_accessibility_settings_set_ui_scale (self->settings, self->ui_scale);
}

static void
on_reduce_motion_toggled (LrgWidget *toggle,
                          gpointer   user_data)
{
    LpAccessibilityPanel *self = LP_ACCESSIBILITY_PANEL (user_data);

    self->reduce_motion = lrg_toggle_get_active (LRG_TOGGLE (toggle));
    lrg_accessibility_settings_set_reduce_motion (self->settings, self->reduce_motion);
}

static void
lp_accessibility_panel_dispose (GObject *object)
{
    LpAccessibilityPanel *self = LP_ACCESSIBILITY_PANEL (object);

    g_clear_object (&self->settings);

    G_OBJECT_CLASS (lp_accessibility_panel_parent_class)->dispose (object);
}

static void
lp_accessibility_panel_init (LpAccessibilityPanel *self)
{
    self->colorblind_index = 0;
    self->high_contrast = FALSE;
    self->ui_scale = 1.0f;
    self->reduce_motion = FALSE;
}

static void
lp_accessibility_panel_class_init (LpAccessibilityPanelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    object_class->dispose = lp_accessibility_panel_dispose;
}

LpAccessibilityPanel *
lp_accessibility_panel_new (LrgAccessibilitySettings *settings)
{
    LpAccessibilityPanel *self;
    LrgWidget *vbox;
    LrgWidget *row;
    gint i;

    g_return_val_if_fail (LRG_IS_ACCESSIBILITY_SETTINGS (settings), NULL);

    self = g_object_new (LP_TYPE_ACCESSIBILITY_PANEL, NULL);
    self->settings = g_object_ref (settings);

    /* Load current settings */
    self->colorblind_index = (gint)lrg_accessibility_settings_get_colorblind_type (settings);
    self->high_contrast = lrg_accessibility_settings_get_high_contrast (settings);
    self->ui_scale = lrg_accessibility_settings_get_ui_scale (settings);
    self->reduce_motion = lrg_accessibility_settings_get_reduce_motion (settings);

    /* Create layout */
    vbox = lrg_vbox_new (16);
    lrg_widget_add_child (LRG_WIDGET (self), vbox);

    /* === Vision Section === */
    lrg_widget_add_child (vbox, lrg_label_new ("Vision", "heading"));

    /* Colorblind mode */
    row = lrg_hbox_new (8);
    self->colorblind_label = lrg_label_new ("Colorblind Mode", "body");
    lrg_widget_set_expand (self->colorblind_label, TRUE, FALSE);
    lrg_widget_add_child (row, self->colorblind_label);

    self->colorblind_selector = lrg_selector_new ();
    for (i = 0; COLORBLIND_OPTIONS[i] != NULL; i++)
    {
        lrg_selector_add_option (LRG_SELECTOR (self->colorblind_selector),
                                 COLORBLIND_OPTIONS[i]);
    }
    lrg_selector_set_selected_index (LRG_SELECTOR (self->colorblind_selector),
                                     self->colorblind_index);
    g_signal_connect (self->colorblind_selector, "selection-changed",
                      G_CALLBACK (on_colorblind_changed), self);
    lrg_widget_add_child (row, self->colorblind_selector);
    lrg_widget_add_child (vbox, row);

    /* High contrast */
    row = lrg_hbox_new (8);
    self->high_contrast_label = lrg_label_new ("High Contrast", "body");
    lrg_widget_set_expand (self->high_contrast_label, TRUE, FALSE);
    lrg_widget_add_child (row, self->high_contrast_label);

    self->high_contrast_toggle = lrg_toggle_new ();
    lrg_toggle_set_active (LRG_TOGGLE (self->high_contrast_toggle), self->high_contrast);
    g_signal_connect (self->high_contrast_toggle, "toggled",
                      G_CALLBACK (on_high_contrast_toggled), self);
    lrg_widget_add_child (row, self->high_contrast_toggle);
    lrg_widget_add_child (vbox, row);

    /* UI Scale */
    row = lrg_hbox_new (8);
    self->ui_scale_label = lrg_label_new ("UI Scale", "body");
    lrg_widget_set_expand (self->ui_scale_label, TRUE, FALSE);
    lrg_widget_add_child (row, self->ui_scale_label);

    self->ui_scale_slider = lrg_slider_new (0.75f, 2.0f);
    lrg_slider_set_value (LRG_SLIDER (self->ui_scale_slider), self->ui_scale);
    lrg_slider_set_step (LRG_SLIDER (self->ui_scale_slider), 0.25f);
    g_signal_connect (self->ui_scale_slider, "value-changed",
                      G_CALLBACK (on_ui_scale_changed), self);
    lrg_widget_add_child (row, self->ui_scale_slider);
    lrg_widget_add_child (vbox, row);

    /* === Motion Section === */
    lrg_widget_add_child (vbox, lrg_label_new ("Motion", "heading"));

    /* Reduce motion */
    row = lrg_hbox_new (8);
    self->reduce_motion_label = lrg_label_new ("Reduce Motion", "body");
    lrg_widget_set_expand (self->reduce_motion_label, TRUE, FALSE);
    lrg_widget_add_child (row, self->reduce_motion_label);

    self->reduce_motion_toggle = lrg_toggle_new ();
    lrg_toggle_set_active (LRG_TOGGLE (self->reduce_motion_toggle), self->reduce_motion);
    g_signal_connect (self->reduce_motion_toggle, "toggled",
                      G_CALLBACK (on_reduce_motion_toggled), self);
    lrg_widget_add_child (row, self->reduce_motion_toggle);
    lrg_widget_add_child (vbox, row);

    return self;
}

void
lp_accessibility_panel_apply (LpAccessibilityPanel *self)
{
    g_return_if_fail (LP_IS_ACCESSIBILITY_PANEL (self));

    /* Settings are applied immediately via callbacks */
    /* This method can be used for additional application logic */

    /* Save settings to persistent storage */
    lrg_settings_group_save (LRG_SETTINGS_GROUP (self->settings));
}
