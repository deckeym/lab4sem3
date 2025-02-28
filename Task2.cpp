#include <iostream> // Подключение библиотеки для ввода/вывода
#include <vector> // Подключение библиотеки для работы с векторами
#include <thread> // Подключение библиотеки для работы с потоками
#include <mutex> // Подключение библиотеки для работы с мьютексами
#include <chrono> // Подключение библиотеки для работы с временем
#include <string> // Подключение библиотеки для работы со строками
#include <map> // Подключение библиотеки для работы с ассоциативными контейнерами
#include <numeric> // Подключение библиотеки для числовых операций
#include <random> // Подключение библиотеки для генерации случайных чисел
#include <atomic> // Подключение библиотеки для атомарных операций

using namespace std; // Использование пространства имен std

// Структура для хранения информации о занятии
struct Lesson {
    string teacher; // ФИО преподавателя
    int week; // Номер недели
    int day; // День недели проведения занятия
    int start_hour; // Время начала занятия (часы)
    int duration; // Длительность занятия (часы)
};

mutex cout_mutex; // Глобальный мьютекс для синхронизации вывода

// Функция для подсчёта рабочих часов преподавателя в заданную неделю
int calculateHours(const vector<Lesson>& schedule, const string& teacher, int week) {
    int total_hours = 0; // Общее количество часов
    for (const auto& lesson : schedule) {
        if (lesson.teacher == teacher && lesson.week == week) {
            total_hours += lesson.duration; // Добавляем продолжительность занятия
        }
    }
    return total_hours; // Возвращаем общее количество часов
}

// Оптимизированная многопоточная обработка
int parallelCalculateHours(const vector<Lesson>& schedule, const string& teacher, int week) {
    unsigned int num_threads = min(thread::hardware_concurrency(), static_cast<unsigned int>(schedule.size() / 100)); // Оптимизация количества потоков
    vector<thread> threads;
    atomic<int> total_hours = 0; // Атомарная переменная для избежания блокировок
    size_t chunk_size = schedule.size() / num_threads;
    
    for (unsigned int i = 0; i < num_threads; ++i) {
        auto begin = schedule.begin() + i * chunk_size;
        auto end = (i == num_threads - 1) ? schedule.end() : begin + chunk_size;
        
        threads.emplace_back([begin, end, &teacher, week, &total_hours]() {
            int local_hours = 0;
            for (auto it = begin; it != end; ++it) {
                if (it->teacher == teacher && it->week == week) {
                    local_hours += it->duration;
                }
            }
            total_hours += local_hours;
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    return total_hours.load();
}

// Функция для генерации случайного расписания
vector<Lesson> generateRandomSchedule(int num_lessons) {
    vector<string> teachers = {"Иванов И.И.", "Петров П.П.", "Сидоров С.С.", "Кузнецов А.А."}; // Список преподавателей
    vector<Lesson> schedule; // Вектор для хранения занятий
    random_device rd;
    mt19937 gen(rd()); // Генератор случайных чисел
    uniform_int_distribution<> week_dist(1, 4); // Генерация номера недели (1-4)
    uniform_int_distribution<> day_dist(1, 7); // Генерация дня недели (1-7)
    uniform_int_distribution<> hour_dist(8, 18); // Генерация времени начала (8-18 часов)
    uniform_int_distribution<> duration_dist(1, 3); // Генерация продолжительности (1-3 часа)
    uniform_int_distribution<> teacher_dist(0, teachers.size() - 1); // Генерация преподавателя
    
    for (int i = 0; i < num_lessons; ++i) {
        schedule.push_back({teachers[teacher_dist(gen)], week_dist(gen), day_dist(gen), hour_dist(gen), duration_dist(gen)});
    }
    return schedule; // Возвращаем сгенерированное расписание
}

int main() {
    string teacher = "Иванов И.И."; // Выбранный преподаватель
    int week = 1; // Выбранная неделя
    
    // Малый объем данных
    auto small_schedule = generateRandomSchedule(1000);
    auto start = chrono::high_resolution_clock::now();
    int small_single_thread_result = calculateHours(small_schedule, teacher, week);
    auto end = chrono::high_resolution_clock::now();
    auto small_single_thread_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    start = chrono::high_resolution_clock::now();
    int small_multi_thread_result = parallelCalculateHours(small_schedule, teacher, week);
    end = chrono::high_resolution_clock::now();
    auto small_multi_thread_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    cout << "Малый объем данных:\n";
    cout << "Результат (один поток): " << small_single_thread_result << " часов, время: " << small_single_thread_time << " мкс\n";
    cout << "Результат (многопоточность): " << small_multi_thread_result << " часов, время: " << small_multi_thread_time << " мкс\n";
    
        // Большой объем данных
    auto large_schedule = generateRandomSchedule(100000);
    start = chrono::high_resolution_clock::now();
    int large_single_thread_result = calculateHours(large_schedule, teacher, week);
    end = chrono::high_resolution_clock::now();
    auto large_single_thread_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    start = chrono::high_resolution_clock::now();
    int large_multi_thread_result = parallelCalculateHours(large_schedule, teacher, week);
    end = chrono::high_resolution_clock::now();
    auto large_multi_thread_time = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    cout << "Большой объем данных:\n";
    cout << "Результат (один поток): " << large_single_thread_result << " часов, время: " << large_single_thread_time << " мкс\n";
    cout << "Результат (многопоточность): " << large_multi_thread_result << " часов, время: " << large_multi_thread_time << " мкс\n";
    
    return 0; // Завершение программы
}
