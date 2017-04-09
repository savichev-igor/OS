# Плагин для gzip
Данный плагин создаёт из распакованного файла разрежённый файл.

## Запуск
~~~bash
make compile
sudo chmox +x main
gzip -cd file.gz | ./main sparce_file
~~~

### Надежда
Здесь автор в ней не нуждается =).
