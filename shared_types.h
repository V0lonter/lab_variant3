#pragma once

#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Вузол таблиці: пара (x_i, y_i).
struct Point {
    double x;
    double y;
};

// Спільні вхідні дані для обох алгоритмів (Варіант 3).
// Передаються як std::shared_ptr<const InputData>: один об'єкт, читають обидва.
struct InputData {
    std::vector<Point> points;  // вузли, строго зростаючі за x
    double x0;                  // точка, у якій шукаємо значення функції
};

// Результат роботи алгоритму. Усі поля відкриті, тому працюють structured bindings:
//   auto [method, value, error, ops, time] = *result;
struct Result {
    std::string method;     // назва методу
    double value;           // інтерпольоване значення y(x0)
    double error_estimate;  // оцінка похибки (див. коментарі в student_a.cpp / student_b.cpp)
    long long operations;   // кількість арифметичних операцій (+ - * /) основного обчислення
    double time_us;         // час основного обчислення, мкс
};

// Студент А: інтерполяція Лагранжа.
std::unique_ptr<Result> calculateA(std::shared_ptr<const InputData> data);

// Студент Б: кубічна сплайн-інтерполяція.
std::unique_ptr<Result> calculateB(std::shared_ptr<const InputData> data);

// ---------------------------------------------------------------------------
// Спільні допоміжні функції (обидва алгоритми перевіряють вхід однаково).
// ---------------------------------------------------------------------------

// Перевірка вхідних даних: >= 3 вузлів, x строго зростають, x0 лежить у межах таблиці.
inline void validate_input(const std::shared_ptr<const InputData>& data) {
    if (!data) {
        throw std::invalid_argument("Вхідні дані відсутні (nullptr)");
    }
    const auto& p = data->points;
    if (p.size() < 3) {
        throw std::invalid_argument("Потрібно щонайменше 3 вузли");
    }
    for (std::size_t i = 1; i < p.size(); ++i) {
        if (!(p[i].x > p[i - 1].x)) {
            throw std::invalid_argument("Вузли мають бути впорядковані за x строго за зростанням");
        }
    }
    if (!(data->x0 >= p.front().x && data->x0 <= p.back().x)) {
        throw std::invalid_argument("Точка x0 виходить за межі таблиці (екстраполяція не підтримується)");
    }
}

// Індекс внутрішнього (не крайнього) вузла, найближчого до x0.
// Обидва методи оцінюють похибку однаково: прибирають цей вузол,
// перебудовують інтерполянт і дивляться, наскільки змінилося значення в x0.
inline std::size_t nearest_interior_index(const std::vector<Point>& p, double x0) {
    std::size_t best = 1;
    for (std::size_t i = 2; i + 1 < p.size(); ++i) {
        if (std::abs(p[i].x - x0) < std::abs(p[best].x - x0)) {
            best = i;
        }
    }
    return best;
}
