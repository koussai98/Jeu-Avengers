#!/bin/bash
# ──────────────────────────────────────────────────────────────────────────────
# setup.sh – Installe les dépendances SDL2 et crée les assets manquants
# Usage : bash setup.sh
# ──────────────────────────────────────────────────────────────────────────────

set -e
echo "=== Installation des bibliothèques SDL2 ==="
sudo apt-get update -qq
sudo apt-get install -y \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-ttf-dev \
    libsdl2-mixer-dev \
    python3-pil   # pour générer les images placeholder

echo ""
echo "=== Création des dossiers d'assets ==="
mkdir -p assets/images assets/sounds assets/fonts

# ── Générer des images placeholder avec Python PIL ────────────────────────
python3 - <<'EOF'
from PIL import Image, ImageDraw, ImageFont
import os

COLORS = {
    "background1.png": (30,  80, 140),
    "background2.png": (20,  60, 100),
    "background3.png": (50,  30, 100),
    "background4.png": (20,  20,  60),
}

for fname, color in COLORS.items():
    path = os.path.join("assets", "images", fname)
    if not os.path.exists(path):
        img = Image.new("RGB", (800, 600), color)
        draw = ImageDraw.Draw(img)
        draw.text((350, 280), fname, fill=(200, 200, 200))
        img.save(path)
        print(f"  Créé : {path}")
    else:
        print(f"  Existe déjà : {path}")
EOF

# ── Télécharger une police libre de droits ───────────────────────────────
FONT_PATH="assets/fonts/font.ttf"
if [ ! -f "$FONT_PATH" ]; then
    echo ""
    echo "=== Téléchargement de la police DejaVuSans ==="
    # DejaVu est incluse dans la plupart des systèmes Ubuntu
    SYSTEM_FONT=$(fc-list | grep -i "DejaVuSans.ttf" | head -1 | cut -d: -f1)
    if [ -n "$SYSTEM_FONT" ]; then
        cp "$SYSTEM_FONT" "$FONT_PATH"
        echo "  Police copiée depuis : $SYSTEM_FONT"
    else
        echo "  [WARN] Police DejaVuSans introuvable."
        echo "  Copiez manuellement un fichier .ttf dans assets/fonts/font.ttf"
    fi
else
    echo "  Police déjà présente : $FONT_PATH"
fi

# ── Créer des fichiers son vides (silence WAV/OGG) si absents ────────────
echo ""
echo "=== Vérification des fichiers son ==="
for f in assets/sounds/click.wav assets/sounds/music_menu.ogg assets/sounds/music_options.ogg; do
    if [ ! -f "$f" ]; then
        echo "  [WARN] Absent : $f  (ajoutez votre propre fichier audio)"
    else
        echo "  OK : $f"
    fi
done

echo ""
echo "============================================================"
echo "  Setup terminé !"
echo "  Placez vos fichiers audio dans : assets/sounds/"
echo "    - click.wav          (son de survol bouton)"
echo "    - music_menu.ogg     (musique menu principal)"
echo "    - music_options.ogg  (musique sous-menus)"
echo ""
echo "  Pour compiler et lancer :"
echo "    make run"
echo "============================================================"
