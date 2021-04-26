# Huffman archiver

Программа, выполняющая сжатие двухпроходным алгоритмом Хаффмана.

   * Наибольший размер входного файла — 5MB.

   * Реализация выполнена в объектно-ориентированном стиле (например, включает классы `TreeNode`, `HuffTree`, `HuffmanArchiver`).
   * Для хранения элементов, там, где это уместно, должна быть использована библиотека STL (например, `std::vector`, а не динамический массив).
   * Для автоматического тестирования используется библиотека `doctest`.
   
   **Параметры командной строки.** 
   Значение параметра (если есть) указывается через пробел.
   * `-c`: архивирование
   * `-u`: разархивирование
   * `-f`, `--file <путь>`: имя входного файла
   * `-o`, `--output <путь>`: имя результирующего файла
   
   **Вывод на экран.**
   Программа выводит на экран статистику сжатия/распаковки: размер исходных данных, размер полученных данных
   и размер, который был использован для хранения вспомогательных данных в выходном файле.
   Все размеры в байтах.

   Например:
   ```
   $ ./huffman -c -f input.txt -o output.bin
   15678
   6172
   482
   ```

   Размер исходного файла (исходные данные): 15678 байт, размер сжатых данных (без дополнительной информации):
   6172 байта, размер дополнительных данных: 482 байта. Размер всего сжатого файла: 6172 + 482 = 6654 байта.
   ```
   $ ./huffman -u -f result.bin -o myfile_new.txt
   6172
   15678
   482
   ```
   Размер распакованного файла (полученные данные): 15678 байт, размер сжатых данных (без дополнительной информации):
   6172 байта, размер дополнительных данных: 482 байта. Размер всего исходного сжатого файла: 6172 + 482 = 6654 байта.

   * `Makefile`:
	 * цель по умолчанию собирает исполняемый файл `huffman` и объектные файлы в директорию `obj` (создается при сборке, если не существует)
	 * цель `test` собирает исполняемый файл `huffman_test` и объектные файлы в директорию `obj`
	 * цель `clean` очищает директорию `obj` и удаляет собранные исполняемые файлы
