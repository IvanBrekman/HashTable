# HashTable
Хеш-таблица с закрытой адресацией для хранения строк и дальнейшего поиска по таблице.
<br><br><br>

## Введение
Целью данной работы является разработка словаря для хранения массива строк и дальнейшего поиска по словарю. В качестве словаря будет использоваться хеш-таблица со списками, реализованными под хранение строк.

После реализации структуры данных предстоит провести анализ и выбрать хеш-функцию, которая лучше всего подойдет для нашего use case. Затем нашей задачей будет замерить время работы хеш-таблицы и попытаться максимально ее оптимизировать, стараясь при этом сохранить переносимость программы на другие устройства.

Приятного чтения, и да прибудет с вами Котик!

![image](https://sun1-17.userapi.com/impg/ParUUs2WiFcnxg14QOSIok7RerIBC1HosEMjUg/snp42wDVjSI.jpg?size=200x0&quality=88&crop=5,4,890,890&sign=1b61ea57e100650d16e7f9cc5d4b1fea&c_uniq_tag=o5bASAgP_RWciRPxU2cCCZCFXGnNvufio-_m3OZmUno&ava=1)
<br><br><br>

## Реализация таблицы
Хеш-таблица не самая простая структура данных, которая работает с объектами (например со списками) через указатели. Да и сами строки в сущности являются указателями, которые надо создавать, хранить, возможно, очищать, не говоря уже о том, что в процессе оптимизаций можно запросто что-то сломать или испортить и потратить часы на дебаг очередного `Segmentation Fault`.

Предостерегая эти ошибки, сделаем жертву временем Богу Дебага, написав защищенные таблицу и список. Это означает, что при реализации будут добавлены поля и функции, которые отвечают за проверку корректного состояния структур данных и вывода подробной информации обо всех полях объекта в случае ошибки или при желании.

Из этих соображений имеем следующую структуру таблицы.
```c++
typedef char* item_t;

const long unsigned INIT_CANARY  = 0x5AFEA2EA; // SAFE AREA

struct HashInfo {
    const char* type = nullptr;
    const char* name = nullptr;
    const char* file = nullptr;
    const char* func = nullptr;
          int   line = 0;
};

struct HashTable {
    unsigned long long _lcanary = INIT_CANARY;
    validate_level_t   _vlevel  = validate_level_t::NO_VALIDATE;
    HashInfo*          _info    = (HashInfo*) poisons::UNINITIALIZED_PTR;

    int                (*_pf)   (item_t* item)                 = (int (*) (item_t*))                poisons::UNINITIALIZED_PTR;
    int                (*_cmp)  (item_t* item1, item_t* item2) = (int (*) (item_t*, item_t*))       poisons::UNINITIALIZED_PTR;
    int                (*_del)  (item_t* item)                 = (int (*) (item_t*))                poisons::UNINITIALIZED_PTR;
    unsigned long long (*_hash) (item_t* item)                 = (unsigned long long (*) (item_t*)) poisons::UNINITIALIZED_PTR;

    List*   data        = (List*)poisons::UNINITIALIZED_PTR;
    int     size        = poisons::UNINITIALIZED_INT;
    int     capacity    = poisons::UNINITIALIZED_INT;

    unsigned long long _rcanary = INIT_CANARY;
};
```

Все поля проинициализируем специальными ядовитыми значениями, чтобы в случае ошибок можно было увидеть, менялись ли значения этих полей с начальных.

Канареек поставим от случайного выхода за границы массива и порчи данных структуры (вовсе не для него).

<p align="center" width="100%">
  <img src="https://cartoonresearch.com/wp-content/uploads/2018/12/putty-tat-tweety-344.jpg"> 
</p>
<br>

Также добавим в структуру уровень валидации для управления проверками при работе таблицы, поле со структурой информации о таблице (откуда она, как называется и пр.), поле для хеш-функции, с помощью которой таблица будет работать, а также поля с функциями для печати, сравнения и удаления элементов, (чтобы не нагружать функции доп аргументами).
<br><br><br>

## Выбор хеш-функции
### Введение
Основа хеш-таблицы - хеш-функция. Хеш-функции в идеале должны для одинаковых объектов выдавать одинаковый ключ, а для разных - разный, однако на практике случаются коллизии (совпадение хешей), поэтому наша задача выбрать хеш-функцию, которая будет иметь меньше всего коллизий.

Для анализа этого введем коэффициент коллизии `collision coef = elems / lists`, где `elems` - кол-во всех элементов в таблице, `lists` - кол-во списков, в которых есть хотя бы 1 элемент. Чем меньше этот коэффициент - тем лучше.

Выбирать будем из 6 функций:

| Название | Описание |
|:----------------|:---------|
| constant_hash | возвращает константу |
| first_letter_hash | возвращает ascii-код первого символа слова |
| symbol_sum_hash | возвращает сумму ascii-кодов символов слова |
| string_len_hash | возвращает длину слова |
| roll_hash | кольцевой хеш |
| crc32_hash | реализация crc32 |

<br>

### Реализация
Будем использовать функцию `test_func_collision`, которая будет создавать таблицу с указанной функцией, заполнять ее данными из указанного файла и считать коэффициент коллизий, выводя результат тестирования в консоль. Также эта функция будет вносить информацию о загруженности таблицы в специальный файл, по которому позже будут строиться графики.

Для замера возьмем текст Шекспира "Ромео и Джульетта". Предварительно с помощью скрипта очистим файл от всех знаков препинания, приведем все слова к нижнему регистру и удалим все повторы, так как наша задача сейчас - оценить качество распределения слов хеш-функцией.

```c++
int test_func_collision(const char* filename, const HashFunc* hash, const char* save_file, int save_from, int save_to) {
    ASSERT_IF(VALID_PTR(filename), "Invalid filename ptr", 0);
    ASSERT_IF(VALID_PTR(hash),     "Invalid hash ptr",     0);

    HashTable*   table   = CREATE_TABLE(table, print_str, cmp_str, del_str, hash->func, validate_level_t::NO_VALIDATE, CAPACITY_VALUES[1]);
    LoadContext* context = load_strings_to_table(table, filename, 1);

    CollisionData* data  = get_collision_info(table);
    
    FILE* dest = open_file(save_file, "a");

    save_to = (save_to == OLD_CAPACITY) ? table->capacity : save_to;

    fprintf(dest, "%s;", hash->func_name);
    for (int i = save_from; i < save_to; i++) {
        List* lst = table->data + i;

        fprintf(dest, "%d", lst->size);
        if (i + 1 < save_to) fprintf(dest, " ");
    }
    fprintf(dest, ";%.3lf\n", data->coef);

    printf("=============== Collision test ===============\n");
    printf("func name:     '%s'\n",   hash->func_name);
    printf("collision coef: %lf\n\n", data->coef);
    printf("finds:          %d\n",    context->finds);
    printf("inserts:        %d\n",    context->inserts);
    printf("fi coef:        %lf\n",   (double)context->finds / context->inserts);
    printf("==============================================\n\n");

    FREE_PTR(data->data,       int);
    FREE_PTR(context->storage, char);

    FREE_PTR(data,    CollisionData);
    FREE_PTR(context, LoadContext);

    table_dtor(table);
    close_file(dest);

    return 1;
}
```
<br>

### Результаты
При запуске программы получаем результаты:

На графиках числа слева означают кол-во элементов в списке, а значения снизу - индекс списка в таблице.
Обратите внимание, что на графиках используется 2 масштаба, которые выбирались, учитывая распределения элементов хеш-функцией.

![image](https://github.com/IvanBrekman/Hash_Table/blob/main/data/graph/graph1.png)
![image](https://github.com/IvanBrekman/Hash_Table/blob/main/data/graph/graph2.png)
![image](https://github.com/IvanBrekman/Hash_Table/blob/main/data/graph/graph3.png)
![image](https://github.com/IvanBrekman/Hash_Table/blob/main/data/graph/graph4.png)
![image](https://github.com/IvanBrekman/Hash_Table/blob/main/data/graph/graph5.png)
![image](https://github.com/IvanBrekman/Hash_Table/blob/main/data/graph/graph6.png)

| Название | Описание | Коэффициент коллизии |
|:----------------|:---------|:----------|
| constant_hash | возвращает константу | 3743.000 |
| first_letter_hash | возвращает ascii-код первого символа слова | 149.720 |
| symbol_sum_hash | возвращает сумму ascii-кодов символов слова | 5.190 |
| string_len_hash | возвращает длину слова | 233.937 |
| roll_hash | кольцевой хеш | 1.895 |
| crc32_hash | реализация crc32 | 1.417 |

<br>

На основе этих данных можем сделать вывод, что `crc32_hash` лучше всех нам подходит. Для дальнейших оптимизаций будем использовать таблицу с этой хеш-функцией.
<br><br><br>

## Оптимизации
### Введение
Наша задача - оценить время работы хеш-таблицы и попытаться ее ускорить. Для этого нужно выбрать, как и что измерять.

Use case подразумевает загрузку большого текста с файла и, дальнейший поиск слов в таблице. При этом кол-во поисков во много раз больше кол-ва вставок. Поэтому будем замерять время загрузки файла в таблицу, при этом наша функция будет проверять есть ли слово в таблице, и если нет, то вставлять его туда. Отношение кол-ва поисков в таблице (`find`) к кол-ву вставок (`insert`) назовем `fi_coef`.

В use case мы подразумеваем использование таблицы для поиска слов, так что кол-во поисков должно быть в разы больше кол-ва вставок. Если после загрузки значение этого коэффициента понадобится увеличить, то будем генерировать рандомные слова и искать их в таблице, при этом замеряя время только поиска в таблице.

Для профилирования программы будем использовать `Callgrind`, а просматривать листинг с помощью `KCacheGrind`, который будет показывать, как много времени от работы программы заняла какая-то функция.
<br>

### Реализация
Выберем уже знакомый нам текст Шекспира. Также с помощью скрипта уберем все знаки препинания и приведем весь текст к нижнему регистру.

Для замеров времени будем использовать функцию `test_table_speed`, которая будет брать данные из указанного файла. Также ей можно передать нужное нам значение `fi_coef`. От запуска к запуску время работы одного и того же кода может меняться, поэтому будем повторять тест `repeats` раз и считать среднее время работы.

При компиляции будем использовать следующие ключи компиляции: `-I . -O2 -g -no-pie -march=native`

```c++
int test_table_speed(const char* filename, int repeats, double fi_coef) {
    ASSERT_IF(VALID_PTR(filename), "Invalid filename ptr", 0);

    uint64_t sum_time = 0;

    LoadContext* context = nullptr;

    for (int i = 0; i < repeats; i++) {
        HashTable* table = CREATE_TABLE(table, print_str, avx_len32_cmp_str, del_str, asm_len32_crc32_hash, validate_level_t::NO_VALIDATE, CAPACITY_VALUES[1]);

        uint64_t start_time = rdtsc();
                context     = load_strings_to_table(table, filename, 0);
        uint64_t end_time   = rdtsc();

        sum_time += end_time - start_time;

        if ((context->inserts * fi_coef) > context->finds) {
            int need_inserts = (int) (context->inserts * fi_coef) - context->finds;

            char* word = (char*) calloc(ALLIGENCE, sizeof(char));
                  word = random_word(word, ALLIGENCE - 1);

            for ( ; need_inserts > 0; need_inserts--) {
                start_time = rdtsc();
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                table_find(table, &word);
                end_time   = rdtsc();

                context->finds++;
                sum_time += (end_time - start_time) / 10;
            }
            FREE_PTR(word, char);
        }

        table_dtor(table);

        FREE_PTR(context->storage, char);
        if (i + 1 < repeats) FREE_PTR(context, LoadContext);
    }
    
    printf("=============== Speed test ===============\n");
    printf("repeats:  %d\n\n", repeats);
    printf("time avg: "); print_time(sum_time / repeats); printf(" ticks\n\n");
    printf("finds:    %d\n",   context->finds);
    printf("inserts:  %d\n",   context->inserts);
    printf("fi coef:  %lf\n",  (double)context->finds / context->inserts);
    printf("==========================================\n\n");

    FREE_PTR(context, LoadContext);

    return 1;
}
```
<br>

Теперь отключаем все проверки, чтобы они не влияли на время работы. `table_error` отключить очень просто - достаточно передать в таблицу уровень валидации `validate::NO_VALIDATE`, при котором в таблице отключаются проверки.

Также надо отключить конструкцию `ASSERT_IF`.

```c++
#define ASSERT_IF(cond, text, ret) do {                                             \
    assert((cond) && text);                                                         \
    if (!(cond)) {                                                                  \
        PRINT_WARNING(text "\n");                                                   \
        errno = -1;                                                                 \
        return ret;                                                                 \
    }                                                                               \
} while(0)
```

Как видно, просто отключить `assert` не достаточно, поэтому добавим define `NO_CHECKS` при котором будем отключать весь `ASSERT_IF`. Таким образом получим аналог `#define NDEBUG`.

С самого начала имеем результат:
```
=============== Speed test ===============
repeats:  100

time avg: 0.004815 sec

finds:    37430
inserts:  3743
fi coef:  10.000000
==========================================
```

Приступим к оптимизациям!

![hippo](https://github.com/IvanBrekman/Hash_Table/blob/main/data/images/cat_proger_gif.gif)
<br>

### Оптимизация 1
Анализируем вывод KCacheGrind.
![image](https://github.com/IvanBrekman/Hash_Table/blob/main/data/images/opt2_split.png)

Сверху списка замечаем вызовы `random`, которые нас не интересуют, так как они используются для генерации рандомных слов для дополнительных поисков в таблице и на ее скорость никак не влияют. После них видим функцию `calloc_s`, которая занимается выделением памяти под указатели.

Внимательно изучим вкладку `Callers`. В ней мы видим, что эту функцию вызывают `test_table_speed` и `load_strings_to_table`, которые выполняют всю работу и в оптимизации не нуждаются, `table_ctor` и `list_ctor`, которые инициализируют объекты этих структур. Наконец функции `get_text_from_file` и `split`, занимающиеся непосредственно загрузкой текста с файла и разбиением его на слова.

Разберемся немного в их работе. `get_text_from_file` загрузочная функция, которая сгружает в буфер весь текст с файла, после чего проходится по буферу, раcставляя указатели на начала строк, считая параллельно длину этих строк и их кол-во, а также заменяет символы `\n` на `\0`, получая таким образом структуру `Text`. Далее функция `split` проходится по каждой строке и, заменяя пробел на `\0`, сгружает указатели на начала слов в массив, который позже возвращает.

<p align="center" width="100%">
  <img src="http://risovach.ru/upload/2013/09/mem/nu-davay-rasskazhi-mne_30454451_orig_.jpeg"> 
</p>
<br>

Несложно заметить, что функции выполняют похожую работу, при этом вся информативность структуры `Text` нам даже не нужна. Поэтому можно переделать логику `load_strings_to_table`, загружая "сырой" текст в буфер и проходя по нему, расставлять указатели на начала слов и менять пробельные символы на `\0`.

```c++
LoadContext* load_strings_to_table(HashTable* table, const char* filename, int force_insert) {
    ASSERT_OK_HASHTABLE(table,     "Check before load strings_to_table func", nullptr);
    ASSERT_IF(VALID_PTR(filename), "Invalid file ptr",                        nullptr);

    char* data = get_raw_text(filename);

    LoadContext* ctx = NEW_PTR(LoadContext, 1);
    *ctx = { 0, 0, data };

    for (int i = 0; data[i] != '\0'; i++) {
        if (isspace(data[i])) {
            data[i] = '\0';
        } else {
            char* word = data + i;

            while (!isspace(data[i]) && data[i] != '\0') i++;
            data[i] = '\0';

            if (force_insert || table_find(table, word) == NOT_FOUND) {
                table_add(table, word);
                ctx->inserts++;
            }

            ctx->finds += (!force_insert);
        }
    }

    ASSERT_OK_HASHTABLE(table, "Check load to table", nullptr);

    return ctx;
}
```
<br>

Таким образом, последовав советам мудреца ниже, мы упростили логику программы и показали свою приверженность принципу `KISS`.

<p align="center" width="100%">
  <img src="https://gettingclose.ru/wp-content/uploads/2020/08/uslozhnat.jpg"> 
</p>
<br>

Упростить, упростили, а ускорили ли? Смотрим.
```
=============== Speed test ===============
repeats:  100

time avg: 0.003480 sec

finds:    37430
inserts:  3743
fi coef:  10.000000
==========================================
```

Оптимизация сделала таблицу быстрее в 1.4 раза. Отличный результат на таком времени работы! Продолжаем.
<br><br>

### Оптимизация 2
Анализируем новый вывод KCacheGrind.
![image](https://github.com/IvanBrekman/HashTable/blob/main/data/images/opt2_hash_x10.png)

В листинге мы замечаем функцию `load_strings_to_table`. Это загрузочная функция, а значит значительное время работы программы занимает загрузка данных в таблицу, что говорит нам о том, что `fi_coef` слишком маленький, так как use case подразумевает нагрузку на поиск слов в разы больше, чем на загрузку. Увеличим его значение до 100 и снова замерим время работы.

Текущее время.
```
=============== Speed test ===============
repeats:  100

time avg: 49_422_104 ticks

finds:    374300
inserts:  3743
fi coef:  100.000000
==========================================
```

Вывод KCacheGrind.
![image](https://github.com/IvanBrekman/HashTable/blob/main/data/images/opt2_hash.png)

Начнем с оптимизации `crc32_hash`. Возможностей у процессора куда больше, чем может казаться. Так, сейчас мы считаем хеш всех слов по одной букве, хотя могли бы сразу по 8, используя всю мощь регистров. Единственное препятствие - как понять, когда остановиться, ведь слова могут быть разной длины? Раньше признаком окончания был нулевой символ, символизирующий окончание строки, но что делать, если мы считаем сразу 8 символов? Проверять каждый раз среди этих 8 символов `\0` очень не хочется. Здесь мы воспользуемся особенностью наших целевых данный. Это слова, а много ли слов больше 32 букв вы знаете? Я нет. Пользуясь этим знанием, мы можем договориться всегда считать 4 раза по 8 символов, ускорив таким образом подсчет хеша. Однако это требует от нас дополнительных расходов в виде выравнивания всех слов по 32 байта, ведь если в этих ячейках окажутся данные кроме подсчитываемого слова, то у одинаковых слов, лежащих в разных местах будет разный хеш. Такое недопустимо!

Для реализации нашей затеи будем использовать ассемблерную вставку. Она сильно ускорит время работы функции, но понизит переносимость программы, так как на разных машинах может использоваться разный ассемблер.

Таким образом имеем новую реализацию хеш-функции, использующую процессорную команду `crc32`.

```c++
unsigned long long asm_len32_crc32_hash(item_t* item) {
    char* string = *item;

    unsigned long long hash = 0;

    __asm__(
        ".intel_syntax noprefix     \n\t"

        "mov rcx, 4                 \n\t"
        "xor %[ret_val], %[ret_val] \n\t"

        "calc_hash:                 \n\t"
            "mov rax, [%[arg_val]]  \n\t"

            "crc32 %[ret_val], rax  \n\t"
            "add %[arg_val], 8      \n\t"
            "loop calc_hash         \n\t"

        ".att_syntax prefix         \n\t"

        : [ret_val]"=r"(hash)
        : [arg_val]"S"(string)
        : "%rax", "%rcx"
    );

    return hash;
}
```

Есть ли от этого польза?
```
=============== Speed test ===============                               =============== Speed test ===============
repeats:  100                                                            repeats:  100
                                                           __   
time avg: 49_422_104 ticks                                    \          time avg: 13_521_758 ticks
                                                    ----------|          
finds:    374300                                           __ /          finds:    374300
inserts:  3743                                                           inserts:  3743
fi coef:  100.000000                                                     fi coef:  100.000000
==========================================                               ==========================================
```

Учитывая накладные расходы мы ускорили программу в 3.65 раз... используя ассемблер! И что вы на это скажете, хейтеры ассемблера?

![hippo](https://journals.ru/smile/users/13/125/12478.gif)
<br><br>

### Оптимизация 3
Смотрим в наш любимый вывод KCacheGrind.
![image](https://github.com/IvanBrekman/HashTable/blob/main/data/images/opt3_strcmp.png)

Как видно нагрузка хеш-функции сильно упала, так что теперь время заняться функцией `list_find`.

Становится понятно, что основное время работы функции уходит на `strcmp`. Займемся ее улучшением. Здесь у нас будет схожая логика рассуждений. Мы уже имеем данные, которые точно укладываются в 32 байта. Будем использовать не ассемблерную вставку, а реализацию через `intrinsic` функции. Пользуясь 256-разрядными регистрами, которые содержат `256/8=32` байта мы сможем за 1 команду сравнить сразу 2 полных слова! Это ли не магия?

Вашему вниманию новая функция для сравнения строк.

```c++
int strcmp_avx_32len(item_t* item1, item_t* item2) {
    __m256i str1 = _mm256_lddqu_si256((const __m256i*) *item1);
    __m256i str2 = _mm256_lddqu_si256((const __m256i*) *item2);

    __m256i res  = _mm256_cmpeq_epi8(str1, str2);

    int cmp_res  = _mm256_movemask_epi8(res);

    return cmp_res != -1;
}
```

Достаточно простой и, как мне кажется, понятный код (в сравнении с соответствующей ассемблерной вставкой уж точно). Работает все крайне просто. Загружаем строки в `ymm` регистры, сравниваем их значения, после чего сгружаем результат сравнения в переменную типа `int`.

Оценим пользу того, что мы сделали.
```
=============== Speed test ===============                               =============== Speed test ===============
repeats:  100                                                            repeats:  100
                                                           __   
time avg: 13_521_758 ticks                                    \          time avg: 12_049_929 ticks
                                                    ----------|          
finds:    374300                                           __ /          finds:    374300
inserts:  3743                                                           inserts:  3743
fi coef:  100.000000                                                     fi coef:  100.000000
==========================================                               ==========================================
```

Данная оптимизация ускорила программу всего в 1.12 раза. Почему так?

Обратимся еще раз внимательно к листингу программы:
![image](https://github.com/IvanBrekman/HashTable/blob/main/data/images/opt3_strcmp.png)

В нем видно функцию `__strcmp_avx2`, которая вызывается при работе `strcmp`, что говорит нам о том, что базовая реализация `strcmp` и так оптимизирована, поэтому наши оптимизации не так сильно помогли.
<br><br>

### Оптимизация 4
Вы знаете, с чего мы начинаем.
![image](https://github.com/IvanBrekman/HashTable/blob/main/data/images/opt4_list_find.png)

Части `list_find` ускорены, теперь только ускорять саму функцию. Для этого напишем функцию полностью на ассемблере в отдельном файле, причем работу `strcmp` заинлайним внутри функции.

Немного разобравшись в смещениях по адресам получаем файл `list_find.asm`.
```asm
global list_find_asm

section .text


list_find_asm:
                                            ; rdi = list* lst
        mov         rsi, QWORD [rsi]        ; rsi = char* str
        mov         edx, DWORD [rdi+40]     ; rdx = list->size
        mov         rcx, QWORD [rdi+32]     ; rcx = list->data
		
        mov         r8,  0                  ; cycle counter: i = 0

    check_cycle:
        cmp         r8,  rdx                ; cmp(i, lst->size)
        jge         end_cycle               ; if (i >= lst->size) end_cycle
	
	body_cycle:
        mov         r9,  r8
        sal         r9,  3
        add         r9,  rcx                ; r9 = list->data + i
        mov         r9, QWORD [r9]          ; r9 = list->data[i]
		
        vlddqu 	    ymm0, [rsi]
        vlddqu 	    ymm1, [r9]
        vpcmpeqb    ymm2, ymm0, ymm1
        vpmovmskb   r10,  ymm2              ; r10 = cmp(str, list->data[i])
		
        cmp         r10d, 0xFFFFFFFF
        jne         inc_cycle               ; if (r10 != -1 /* if not find */) continue cycle
		
        mov         rax, r8                 ; return i
        jmp         end_find
	
	inc_cycle:
        inc         r8                      ; i++
        jmp         check_cycle
	
	end_cycle:
        mov         eax, -64197             ; return NOT_FOUND;
	
	end_find:
        ret
```

Компилируем и линкуем все файлы, после чего анализируем результат.
```
=============== Speed test ===============                               =============== Speed test ===============
repeats:  100                                                            repeats:  100
                                                           __   
time avg: 12_049_929 ticks                                    \          time avg: 10_479_089 ticks
                                                    ----------|          
finds:    374300                                           __ /          finds:    374300
inserts:  3743                                                           inserts:  3743
fi coef:  100.000000                                                     fi coef:  100.000000
==========================================                               ==========================================
```

В этот раз оптимизация помогла в 1.15 раз. Все еще неплохой результат. Можем продолжать.
<br><br>

### Оптимизация 5
Начинаем как всегда.
![image](https://github.com/IvanBrekman/HashTable/blob/main/data/images/opt5_table_find.png)

Теперь уже в самом верху `table_find`, части которой оптимизированы. Остается повторить прошлую оптимизацию, но на этот раз с другой функцией.

С уже имеющимся опытом написания получаем файл `table_find.asm`
```asm
global table_find

section .text

extern list_find_asm

table_find:
                                            ; rdi = HashTable* table
                                            ; rsi = char** str

        ; ========== HASH FUNC ========== ;
        mov         r8,  0                  ; hash = 0
        mov         r9,  QWORD [rsi]        ; r9   = char* str

        mov         rcx, 4

        calc_hash:
            mov     rax, [r9]

            crc32   r8,  rax
            add     r9,  8
            loop    calc_hash
        ; =============================== ; ; r8   = hash

        mov         rax, r8
        mov         edx, 0

        mov         r10d, DWORD [rdi+68]    ; r10 = table->capacity
        div         r10d                    ; edx = hash % table->capacity

        mov         rdi, QWORD [rdi+56]     ; rdi = table->data
        mov         rax, rdx                ;   ------------------------+
        add         rdx, rdx                ;                           |
        add         rdx, rax                ;                           |
        sal         rdx, 4                  ; rdx *= 48 (sizeof List) <-+
        add         rdi, rdx                ; rdi = table->data + hash
                                            ; rsi = char** str
        call        list_find_asm
        ret                                 ; return list_find(table->data + hash, item);
```

Достаточно внушающе, а помогло ли?
```
=============== Speed test ===============                               =============== Speed test ===============
repeats:  100                                                            repeats:  100
                                                           __   
time avg: 10_479_089 ticks                                    \          time avg: 10_964_645 ticks
                                                    ----------|          
finds:    374300                                           __ /          finds:    374300
inserts:  3743                                                           inserts:  3743
fi coef:  100.000000                                                     fi coef:  100.000000
==========================================                               ==========================================
```

Программа стала работать медленнее на 5%. Получается, что на этом этапе компилятор со своими -O2 оптимизациями справляется лучше, чем наша программа на ассемблере. Что это значит? Это значит, что мы откатываем последнюю оптимизацию, возвращая как было, и заканчиваем оптимизировать таблицу.

### Итоги
Дальнейшие оптимизации уже не имеют смысла, так как дадут слишком мало, при том что потребуют уже немалых усилий, которые, скорее всего, ухудшат переносимость кода.

Поэтому фиксируем результат.

```
=============== Speed test ===============                               =============== Speed test ===============
repeats:  100                                                            repeats:  100
                                                           __   
time avg: 49_422_104 ticks                                    \          time avg: 10_479_089 ticks
                                                    ----------|          
finds:    374300                                           __ /          finds:    374300
inserts:  3743                                                           inserts:  3743
fi coef:  100.000000                                                     fi coef:  100.000000
==========================================                               ==========================================
```

Всеми махинациями мы ускорили нашу таблицу в 4.72 раза. Отличный результат! Можно с чистой совестью отдохнуть и посмотреть мемы про котиков, после чего вернуться к любимому программированию!
<br><br>

<p align="center" width="100%">
  <img src="https://postila.ru/resize?w=480&src=%2Fdata%2F54%2F5a%2Ffe%2F65%2F545afe6582cf59575affc16ed610bc57fc7f7ddc938cbcd4ce5b973ffadbba11.gif"> 
</p>
