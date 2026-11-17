#!/bin/bash

OUTPUT="web_oscilloscope_combined.txt"
FILES=(app.js bridge.py index.html oscilloscope-env-create.sh oscilloscope-env-remove.sh scope.js styles.css)

# Створюємо/очищаємо вихідний файл
> "$OUTPUT"

echo "📦 Об'єднання файлів у $OUTPUT..."
COUNT=0

for f in "${FILES[@]}"; do
    if [[ ! -f "$f" ]]; then
        echo "⚠️  Не знайдено: $f"
        continue
    fi

    echo "   ✅ $f"
    {
        echo "========================================"
        echo "📄 ПОЧАТОК ФАЙЛУ: $f"
        echo "========================================"
        cat "$f"
        echo "========================================"
        echo "🔚 КІНЕЦЬ ФАЙЛУ: $f"
        echo "========================================"
        echo ""
    } >> "$OUTPUT"

    COUNT=$((COUNT + 1))
done

if [[ $COUNT -eq 0 ]]; then
    echo "❌ Жодного файлу не знайдено. Запустіть скрипт у папці з проектом."
    exit 1
fi

echo "✅ Готово! Об'єднано $COUNT файлів."
echo "📄 Файл: $OUTPUT ($(du -h "$OUTPUT" | cut -f1))"
echo "💡 Відкрийте $OUTPUT, скопіюйте ВЕСЬ вміст і вставте сюди."
