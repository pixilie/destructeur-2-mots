echo "Generating compile_commands.json..."
make clean
bear -- make

sed -i 's|/usr|//wsl.localhost/Ubuntu/usr|g' compile_commands.json

sed -i 's|/mnt/d|D:|Ig' compile_commands.json
sed -i 's|/mnt/c|C:|Ig' compile_commands.json

sed -i 's|"-Iinclude",|"-Iinclude", "-I//wsl.localhost/Ubuntu/usr/include",|g' compile_commands.json

sed -i 's|//wsl.localhost/Ubuntu/usr/bin/gcc|gcc|g' compile_commands.json

echo "JSON file updated successfully."
