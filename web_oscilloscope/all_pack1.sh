
# Створити єдиний файл з маркерами
{
  echo "📄 ====== APP.JS ======"
  cat app.js
  echo -e "\n📄 ====== SCOPE.JS ======"
  cat scope.js
  echo -e "\n📄 ====== BRIDGE.PY ======"
  cat bridge-binary.py
  echo -e "\n📄 ====== КІНЕЦЬ ======"
} > release_code.txt

