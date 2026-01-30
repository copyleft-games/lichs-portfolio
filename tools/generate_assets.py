#!/usr/bin/env python3
"""
Generate minimalist/abstract visual assets for Lich's Portfolio.
Creates simple geometric icons and UI textures programmatically.

Copyright (C) 2026 Zach Podbielniak
SPDX-License-Identifier: AGPL-3.0-or-later
"""

import os
import math
from PIL import Image, ImageDraw, ImageFont

# Color palette from PLAN.md theme
COLORS = {
    'primary': (45, 27, 78),        # Deep purple #2d1b4e
    'secondary': (232, 224, 213),   # Bone white #e8e0d5
    'accent': (201, 162, 39),       # Gold #c9a227
    'background': (10, 10, 15),     # Near black #0a0a0f
    'text': (212, 208, 200),        # Off-white #d4d0c8
    'transparent': (0, 0, 0, 0),
}

# Investment type colors
INVESTMENT_COLORS = {
    'property': (139, 90, 43),      # Brown - earth/land
    'trade': (65, 105, 225),        # Royal blue - commerce
    'financial': (201, 162, 39),    # Gold - money
    'magical': (148, 0, 211),       # Purple - arcane
    'political': (178, 34, 34),     # Dark red - power
    'dark': (20, 20, 25),           # Near black - forbidden
}

# Agent type colors
AGENT_COLORS = {
    'individual': (169, 169, 169),  # Gray - mortal
    'family': (218, 165, 32),       # Goldenrod - dynasty
    'cult': (75, 0, 130),           # Indigo - devotion
    'bound': (0, 100, 0),           # Dark green - undeath
}

ASSETS_DIR = os.path.join(os.path.dirname(__file__), '..', 'assets')


def ensure_dir(path):
    """Create directory if it doesn't exist."""
    os.makedirs(path, exist_ok=True)


def draw_rounded_rect(draw, xy, radius, fill):
    """Draw a rounded rectangle."""
    x0, y0, x1, y1 = xy
    draw.rectangle([x0 + radius, y0, x1 - radius, y1], fill=fill)
    draw.rectangle([x0, y0 + radius, x1, y1 - radius], fill=fill)
    draw.pieslice([x0, y0, x0 + 2*radius, y0 + 2*radius], 180, 270, fill=fill)
    draw.pieslice([x1 - 2*radius, y0, x1, y0 + 2*radius], 270, 360, fill=fill)
    draw.pieslice([x0, y1 - 2*radius, x0 + 2*radius, y1], 90, 180, fill=fill)
    draw.pieslice([x1 - 2*radius, y1 - 2*radius, x1, y1], 0, 90, fill=fill)


def create_investment_icon(name, color, size=64):
    """Create a minimalist investment category icon."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    margin = size // 8
    center = size // 2
    
    if name == 'property':
        # House shape - triangle roof + rectangle base
        points = [
            (center, margin),           # roof peak
            (margin, center),           # roof left
            (size - margin, center),    # roof right
        ]
        draw.polygon(points, fill=color)
        draw.rectangle([margin + 4, center, size - margin - 4, size - margin], fill=color)
        
    elif name == 'trade':
        # Two arrows crossing - exchange
        arrow_width = size // 6
        # Right arrow
        draw.polygon([
            (margin, center - arrow_width),
            (size - margin - arrow_width, center - arrow_width),
            (size - margin - arrow_width, margin),
            (size - margin, center),
            (size - margin - arrow_width, size - margin),
            (size - margin - arrow_width, center + arrow_width),
            (margin, center + arrow_width),
        ], fill=color)
        
    elif name == 'financial':
        # Coin/circle with inner ring
        draw.ellipse([margin, margin, size - margin, size - margin], fill=color)
        inner = margin + size // 8
        draw.ellipse([inner, inner, size - inner, size - inner], fill=COLORS['background'])
        inner2 = inner + size // 12
        draw.ellipse([inner2, inner2, size - inner2, size - inner2], fill=color)
        
    elif name == 'magical':
        # Star/pentagram
        points = []
        for i in range(5):
            angle = math.radians(i * 72 - 90)
            x = center + int((center - margin) * math.cos(angle))
            y = center + int((center - margin) * math.sin(angle))
            points.append((x, y))
            angle2 = math.radians(i * 72 - 90 + 36)
            x2 = center + int((center - margin) * 0.4 * math.cos(angle2))
            y2 = center + int((center - margin) * 0.4 * math.sin(angle2))
            points.append((x2, y2))
        draw.polygon(points, fill=color)
        
    elif name == 'political':
        # Crown shape
        base_y = size - margin - size // 6
        points = [
            (margin, base_y),
            (margin, size - margin),
            (size - margin, size - margin),
            (size - margin, base_y),
            (size - margin - size // 8, margin + size // 4),
            (center + size // 8, base_y - size // 8),
            (center, margin),
            (center - size // 8, base_y - size // 8),
            (margin + size // 8, margin + size // 4),
        ]
        draw.polygon(points, fill=color)
        
    elif name == 'dark':
        # Skull silhouette (simplified)
        # Outer skull shape
        draw.ellipse([margin + 4, margin, size - margin - 4, size - margin - size // 4], fill=color)
        # Jaw
        draw.rectangle([margin + size // 5, size - margin - size // 3, size - margin - size // 5, size - margin], fill=color)
        # Eye sockets (dark)
        eye_size = size // 6
        eye_y = center - size // 8
        draw.ellipse([center - size // 4 - eye_size // 2, eye_y - eye_size // 2,
                      center - size // 4 + eye_size // 2, eye_y + eye_size // 2], fill=COLORS['background'])
        draw.ellipse([center + size // 4 - eye_size // 2, eye_y - eye_size // 2,
                      center + size // 4 + eye_size // 2, eye_y + eye_size // 2], fill=COLORS['background'])
    
    return img


def create_agent_icon(name, color, size=64):
    """Create a minimalist agent type icon."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    margin = size // 8
    center = size // 2
    
    if name == 'individual':
        # Single person silhouette
        head_r = size // 6
        draw.ellipse([center - head_r, margin, center + head_r, margin + head_r * 2], fill=color)
        # Body
        draw.polygon([
            (center - size // 4, margin + head_r * 2 + 2),
            (center + size // 4, margin + head_r * 2 + 2),
            (center + size // 3, size - margin),
            (center - size // 3, size - margin),
        ], fill=color)
        
    elif name == 'family':
        # Three people (family tree)
        head_r = size // 10
        # Parent (center top)
        draw.ellipse([center - head_r, margin, center + head_r, margin + head_r * 2], fill=color)
        # Children (left and right bottom)
        draw.ellipse([margin + size // 8 - head_r, center, margin + size // 8 + head_r, center + head_r * 2], fill=color)
        draw.ellipse([size - margin - size // 8 - head_r, center, size - margin - size // 8 + head_r, center + head_r * 2], fill=color)
        # Connection lines
        draw.line([(center, margin + head_r * 2), (center, center - head_r)], fill=color, width=2)
        draw.line([(margin + size // 8, center - head_r), (size - margin - size // 8, center - head_r)], fill=color, width=2)
        draw.line([(margin + size // 8, center - head_r), (margin + size // 8, center)], fill=color, width=2)
        draw.line([(size - margin - size // 8, center - head_r), (size - margin - size // 8, center)], fill=color, width=2)
        
    elif name == 'cult':
        # Hooded figure with symbol
        hood_w = size // 3
        # Hood shape
        draw.polygon([
            (center, margin),
            (center - hood_w, center + size // 6),
            (center - hood_w + size // 8, size - margin),
            (center + hood_w - size // 8, size - margin),
            (center + hood_w, center + size // 6),
        ], fill=color)
        # Eye glow
        draw.ellipse([center - 3, center - 2, center + 3, center + 4], fill=COLORS['accent'])
        
    elif name == 'bound':
        # Chained figure
        head_r = size // 8
        draw.ellipse([center - head_r, margin + size // 6, center + head_r, margin + size // 6 + head_r * 2], fill=color)
        # Body
        draw.rectangle([center - size // 6, margin + size // 6 + head_r * 2, center + size // 6, size - margin - size // 6], fill=color)
        # Chains (circles)
        chain_r = size // 16
        for i in range(3):
            y = margin + i * size // 4
            draw.ellipse([margin - chain_r, y, margin + chain_r, y + chain_r * 2], outline=COLORS['accent'], width=2)
            draw.ellipse([size - margin - chain_r, y, size - margin + chain_r, y + chain_r * 2], outline=COLORS['accent'], width=2)
    
    return img


def create_ui_panel(size=(256, 128)):
    """Create a UI panel background texture."""
    img = Image.new('RGBA', size, COLORS['background'] + (240,))
    draw = ImageDraw.Draw(img)
    
    # Border
    border = 2
    draw.rectangle([0, 0, size[0] - 1, size[1] - 1], outline=COLORS['accent'], width=border)
    
    # Inner border
    inner = 4
    draw.rectangle([inner, inner, size[0] - inner - 1, size[1] - inner - 1], outline=COLORS['primary'], width=1)
    
    # Corner decorations
    corner_size = 8
    for x in [inner, size[0] - inner - corner_size]:
        for y in [inner, size[1] - inner - corner_size]:
            draw.rectangle([x, y, x + corner_size, y + corner_size], fill=COLORS['accent'])
    
    return img


def create_button(size=(128, 48), state='normal'):
    """Create a button texture."""
    img = Image.new('RGBA', size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    if state == 'normal':
        fill = COLORS['primary']
        outline = COLORS['accent']
    elif state == 'hover':
        fill = (65, 47, 98)  # Lighter purple
        outline = COLORS['accent']
    elif state == 'pressed':
        fill = (25, 17, 58)  # Darker purple
        outline = COLORS['secondary']
    
    # Rounded rectangle button
    draw_rounded_rect(draw, [0, 0, size[0] - 1, size[1] - 1], 6, fill)
    draw.rectangle([0, 0, size[0] - 1, size[1] - 1], outline=outline, width=2)
    
    return img


def create_logo(size=256):
    """Create a simple logo placeholder."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    center = size // 2
    margin = size // 8
    
    # Skull base (representing lich)
    draw.ellipse([margin, margin, size - margin, size - margin - size // 6], fill=COLORS['secondary'])
    
    # Eye sockets with gold glow
    eye_size = size // 5
    eye_y = center - size // 10
    draw.ellipse([center - size // 4 - eye_size // 2, eye_y - eye_size // 2,
                  center - size // 4 + eye_size // 2, eye_y + eye_size // 2], fill=COLORS['background'])
    draw.ellipse([center + size // 4 - eye_size // 2, eye_y - eye_size // 2,
                  center + size // 4 + eye_size // 2, eye_y + eye_size // 2], fill=COLORS['background'])
    
    # Gold coins overlaid (portfolio)
    coin_r = size // 6
    draw.ellipse([center - coin_r - size // 8, size - margin - coin_r * 2,
                  center - size // 8, size - margin], fill=COLORS['accent'])
    draw.ellipse([center - coin_r // 2, size - margin - coin_r * 2,
                  center + coin_r // 2, size - margin], fill=COLORS['accent'])
    draw.ellipse([center + size // 8 - coin_r, size - margin - coin_r * 2,
                  center + size // 8, size - margin], fill=COLORS['accent'])
    
    return img


def create_exposure_meter(size=(200, 24)):
    """Create exposure meter background."""
    img = Image.new('RGBA', size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Background bar
    draw.rectangle([0, 0, size[0] - 1, size[1] - 1], fill=COLORS['background'], outline=COLORS['secondary'], width=1)
    
    # Threshold markers
    thresholds = [0.25, 0.50, 0.75]
    for t in thresholds:
        x = int(t * size[0])
        draw.line([(x, 0), (x, size[1])], fill=COLORS['accent'], width=1)
    
    return img


def create_world_map_background(size=(512, 512)):
    """Create a parchment-style world map background."""
    img = Image.new('RGBA', size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    # Parchment base color
    parchment = (218, 197, 165)
    draw.rectangle([0, 0, size[0] - 1, size[1] - 1], fill=parchment)
    
    # Add some aged spots/variation
    import random
    random.seed(42)  # Reproducible
    for _ in range(100):
        x = random.randint(0, size[0])
        y = random.randint(0, size[1])
        r = random.randint(5, 20)
        shade = random.randint(-20, 20)
        spot_color = (parchment[0] + shade, parchment[1] + shade, parchment[2] + shade - 10)
        spot_color = tuple(max(0, min(255, c)) for c in spot_color)
        draw.ellipse([x - r, y - r, x + r, y + r], fill=spot_color)
    
    # Border
    border = 8
    draw.rectangle([0, 0, size[0] - 1, size[1] - 1], outline=(101, 67, 33), width=border)
    draw.rectangle([border, border, size[0] - border - 1, size[1] - border - 1], outline=COLORS['accent'], width=2)
    
    return img


def create_kingdom_marker(size=32, color=None):
    """Create a kingdom marker for the world map."""
    if color is None:
        color = COLORS['accent']
    
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    center = size // 2
    
    # Castle/tower silhouette
    tower_w = size // 3
    draw.rectangle([center - tower_w // 2, size // 4, center + tower_w // 2, size - 4], fill=color)
    # Battlements
    batt_h = size // 6
    batt_w = size // 8
    draw.rectangle([center - tower_w // 2 - batt_w, size // 4, center - tower_w // 2 + batt_w, size // 4 + batt_h], fill=color)
    draw.rectangle([center + tower_w // 2 - batt_w, size // 4, center + tower_w // 2 + batt_w, size // 4 + batt_h], fill=color)
    # Flag
    draw.polygon([
        (center, 4),
        (center + size // 4, size // 6),
        (center, size // 4),
    ], fill=COLORS['accent'])
    
    return img


def create_region_terrain(terrain_type, size=64):
    """Create terrain tile for a region type."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    if terrain_type == 'coastal':
        # Blue water with land edge
        draw.rectangle([0, 0, size - 1, size - 1], fill=(100, 149, 237))
        draw.polygon([(0, 0), (size // 2, 0), (0, size // 2)], fill=(139, 119, 101))
    elif terrain_type == 'inland':
        # Green plains
        draw.rectangle([0, 0, size - 1, size - 1], fill=(107, 142, 35))
    elif terrain_type == 'mountain':
        # Gray peaks
        draw.rectangle([0, 0, size - 1, size - 1], fill=(119, 136, 153))
        # Mountain shape
        draw.polygon([
            (size // 2, size // 6),
            (size // 6, size - size // 6),
            (size - size // 6, size - size // 6),
        ], fill=(169, 169, 169))
        # Snow cap
        draw.polygon([
            (size // 2, size // 6),
            (size // 3, size // 3),
            (size - size // 3, size // 3),
        ], fill=(255, 255, 255))
    elif terrain_type == 'forest':
        # Dark green with tree shapes
        draw.rectangle([0, 0, size - 1, size - 1], fill=(34, 85, 34))
        # Tree triangles
        for x in [size // 4, size // 2, 3 * size // 4]:
            draw.polygon([
                (x, size // 4),
                (x - size // 6, size - size // 4),
                (x + size // 6, size - size // 4),
            ], fill=(0, 100, 0))
    
    return img


def create_controller_button(button_type, size=64):
    """Create a controller button glyph icon."""
    img = Image.new('RGBA', (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    
    center = size // 2
    margin = size // 8
    
    # Button colors (Xbox style)
    button_colors = {
        'a': (106, 175, 80),    # Green
        'b': (215, 85, 65),     # Red
        'x': (85, 160, 210),    # Blue
        'y': (245, 185, 55),    # Yellow
        'lb': COLORS['secondary'],
        'rb': COLORS['secondary'],
        'lt': COLORS['secondary'],
        'rt': COLORS['secondary'],
        'dpad': COLORS['secondary'],
        'start': COLORS['secondary'],
        'back': COLORS['secondary'],
        'stick_l': COLORS['secondary'],
        'stick_r': COLORS['secondary'],
    }
    
    color = button_colors.get(button_type, COLORS['secondary'])
    
    if button_type in ['a', 'b', 'x', 'y']:
        # Face buttons - circles with letters
        draw.ellipse([margin, margin, size - margin, size - margin], fill=color)
        # Letter (would need font, use simple shape instead)
        letter_size = size // 3
        if button_type == 'a':
            # A shape (triangle pointing up)
            draw.polygon([
                (center, margin + letter_size // 2),
                (center - letter_size // 2, size - margin - letter_size // 2),
                (center + letter_size // 2, size - margin - letter_size // 2),
            ], fill=COLORS['background'])
        elif button_type == 'b':
            # B shape (two bumps)
            draw.ellipse([center - letter_size // 2, center - letter_size // 2,
                          center + letter_size // 2, center], fill=COLORS['background'])
            draw.ellipse([center - letter_size // 2, center,
                          center + letter_size // 2, center + letter_size // 2], fill=COLORS['background'])
            draw.rectangle([center - letter_size // 2, center - letter_size // 2,
                           center - letter_size // 4, center + letter_size // 2], fill=COLORS['background'])
        elif button_type == 'x':
            # X shape (cross)
            lw = letter_size // 4
            draw.line([(center - letter_size // 2, center - letter_size // 2),
                       (center + letter_size // 2, center + letter_size // 2)], 
                      fill=COLORS['background'], width=lw)
            draw.line([(center + letter_size // 2, center - letter_size // 2),
                       (center - letter_size // 2, center + letter_size // 2)], 
                      fill=COLORS['background'], width=lw)
        elif button_type == 'y':
            # Y shape
            lw = letter_size // 4
            draw.line([(center - letter_size // 2, center - letter_size // 2),
                       (center, center)], fill=COLORS['background'], width=lw)
            draw.line([(center + letter_size // 2, center - letter_size // 2),
                       (center, center)], fill=COLORS['background'], width=lw)
            draw.line([(center, center),
                       (center, center + letter_size // 2)], fill=COLORS['background'], width=lw)
    
    elif button_type in ['lb', 'rb']:
        # Bumpers - horizontal pill shape
        h = max(size // 4, 8)
        y0 = center - h // 2
        y1 = center + h // 2
        draw.rounded_rectangle([margin, y0, size - margin, y1], radius=h // 2, fill=color)
    
    elif button_type in ['lt', 'rt']:
        # Triggers - vertical pill shape
        w = max(size // 3, 10)
        x0 = center - w // 2
        x1 = center + w // 2
        draw.rounded_rectangle([x0, margin, x1, size - margin], radius=w // 2, fill=color)
    
    elif button_type == 'dpad':
        # D-pad - cross shape
        dpad_w = size // 4
        draw.rectangle([center - dpad_w // 2, margin, center + dpad_w // 2, size - margin], fill=color)
        draw.rectangle([margin, center - dpad_w // 2, size - margin, center + dpad_w // 2], fill=color)
    
    elif button_type in ['stick_l', 'stick_r']:
        # Analog sticks - circle with dot
        draw.ellipse([margin, margin, size - margin, size - margin], outline=color, width=3)
        dot_r = size // 8
        draw.ellipse([center - dot_r, center - dot_r, center + dot_r, center + dot_r], fill=color)
    
    elif button_type in ['start', 'back']:
        # Menu buttons - small horizontal pill
        h = max(size // 5, 6)
        x0 = margin + size // 6
        x1 = size - margin - size // 6
        draw.rounded_rectangle([x0, center - h, x1, center + h], radius=h, fill=color)
    
    return img


def main():
    """Generate all assets."""
    print("Generating assets for Lich's Portfolio...")
    
    # Investment icons
    icons_dir = os.path.join(ASSETS_DIR, 'textures', 'icons', 'investments')
    ensure_dir(icons_dir)
    for name, color in INVESTMENT_COLORS.items():
        for size in [32, 64, 128]:
            icon = create_investment_icon(name, color, size)
            icon.save(os.path.join(icons_dir, f'{name}_{size}.png'))
            print(f"  Created investment icon: {name}_{size}.png")
    
    # Agent icons
    agents_dir = os.path.join(ASSETS_DIR, 'textures', 'icons', 'agents')
    ensure_dir(agents_dir)
    for name, color in AGENT_COLORS.items():
        for size in [32, 64, 128]:
            icon = create_agent_icon(name, color, size)
            icon.save(os.path.join(agents_dir, f'{name}_{size}.png'))
            print(f"  Created agent icon: {name}_{size}.png")
    
    # UI elements
    ui_dir = os.path.join(ASSETS_DIR, 'textures', 'ui')
    ensure_dir(ui_dir)
    
    # Panels
    for w, h in [(256, 128), (256, 256), (512, 256)]:
        panel = create_ui_panel((w, h))
        panel.save(os.path.join(ui_dir, f'panel_{w}x{h}.png'))
        print(f"  Created UI panel: panel_{w}x{h}.png")
    
    # Buttons
    for state in ['normal', 'hover', 'pressed']:
        btn = create_button(state=state)
        btn.save(os.path.join(ui_dir, f'button_{state}.png'))
        print(f"  Created button: button_{state}.png")
    
    # Exposure meter
    meter = create_exposure_meter()
    meter.save(os.path.join(ui_dir, 'exposure_meter_bg.png'))
    print("  Created exposure meter background")
    
    # Logo
    logo = create_logo(256)
    logo.save(os.path.join(ui_dir, 'logo_256.png'))
    logo_128 = create_logo(128)
    logo_128.save(os.path.join(ui_dir, 'logo_128.png'))
    print("  Created logo")
    
    # World map elements
    world_dir = os.path.join(ASSETS_DIR, 'textures', 'world')
    ensure_dir(world_dir)
    
    # Map background
    map_bg = create_world_map_background((512, 512))
    map_bg.save(os.path.join(world_dir, 'map_background.png'))
    print("  Created world map background")
    
    # Kingdom markers (different colors for different kingdoms)
    kingdom_colors = [
        ('valdris', (178, 34, 34)),    # Dark red
        ('meridia', (65, 105, 225)),   # Royal blue
        ('thornwood', (34, 85, 34)),   # Forest green
        ('ashmark', (128, 128, 128)),  # Gray
        ('sunhold', (218, 165, 32)),   # Goldenrod
        ('neutral', (169, 169, 169)),  # Light gray
    ]
    for name, color in kingdom_colors:
        for s in [24, 32, 48]:
            marker = create_kingdom_marker(s, color)
            marker.save(os.path.join(world_dir, f'kingdom_{name}_{s}.png'))
        print(f"  Created kingdom marker: {name}")
    
    # Terrain tiles
    for terrain in ['coastal', 'inland', 'mountain', 'forest']:
        tile = create_region_terrain(terrain, 64)
        tile.save(os.path.join(world_dir, f'terrain_{terrain}.png'))
        print(f"  Created terrain tile: {terrain}")
    
    # Controller button glyphs
    glyphs_dir = os.path.join(ASSETS_DIR, 'textures', 'glyphs')
    ensure_dir(glyphs_dir)
    
    button_types = ['a', 'b', 'x', 'y', 'lb', 'rb', 'lt', 'rt', 'dpad', 'start', 'back', 'stick_l', 'stick_r']
    for btn in button_types:
        for s in [32, 48, 64]:
            glyph = create_controller_button(btn, s)
            glyph.save(os.path.join(glyphs_dir, f'xbox_{btn}_{s}.png'))
        print(f"  Created controller glyph: {btn}")
    
    # Create asset manifest
    manifest_path = os.path.join(ASSETS_DIR, 'manifest.yaml')
    with open(manifest_path, 'w') as f:
        f.write("""# Lich's Portfolio Asset Manifest
# Generated by tools/generate_assets.py

version: 1
generated: true

textures:
  icons:
    investments:
      - property
      - trade
      - financial
      - magical
      - political
      - dark
    agents:
      - individual
      - family
      - cult
      - bound
  ui:
    - panel_256x128
    - panel_256x256
    - panel_512x256
    - button_normal
    - button_hover
    - button_pressed
    - exposure_meter_bg
    - logo_128
    - logo_256
  world:
    - map_background
    - kingdom_valdris
    - kingdom_meridia
    - kingdom_thornwood
    - kingdom_ashmark
    - kingdom_sunhold
    - kingdom_neutral
    - terrain_coastal
    - terrain_inland
    - terrain_mountain
    - terrain_forest

# All assets are procedurally generated
# No external licenses required
licenses: []
""")
    print(f"  Created manifest: {manifest_path}")
    
    print("\nAsset generation complete!")
    print(f"Assets saved to: {ASSETS_DIR}")


if __name__ == '__main__':
    main()
