#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "shared_types.h"

namespace {

double f_sin(double x) { return std::sin(x); }

// Функція Рунге: на рівномірних вузлах поліном Лагранжа "розгойдується" біля кінців.
double f_runge(double x) { return 1.0 / (1.0 + 25.0 * x * x); }

// Таблиця значень функції f у n рівновіддалених вузлах на [a, b].
std::shared_ptr<const InputData> make_input(double (*f)(double), double a, double b, int n, double x0) {
    std::vector<Point> pts;
    pts.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const double x = a + (b - a) * i / (n - 1);
        pts.push_back({x, f(x)});
    }
    return std::make_shared<const InputData>(InputData{std::move(pts), x0});
}

// --- форматування таблиці (вирівнювання з урахуванням кирилиці в UTF-8) ---
std::size_t utf8_len(const std::string& s) {
    std::size_t n = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) {
            ++n;
        }
    }
    return n;
}

std::string pad(const std::string& s, std::size_t width) {
    const std::size_t len = utf8_len(s);
    return len >= width ? s : s + std::string(width - len, ' ');
}

std::string num(double v, int precision, bool scientific = false) {
    std::ostringstream os;
    os << (scientific ? std::scientific : std::fixed) << std::setprecision(precision) << v;
    return os.str();
}

void print_header() {
    std::cout << pad("Метод", 18) << pad("y(x0)", 14) << pad("Оцінка похибки", 16)
              << pad("Факт. похибка", 15) << pad("Операцій", 10) << "Час, мкс\n"
              << std::string(84, '-') << '\n';
}

void print_row(const std::string& method, double value, double error_estimate,
               double true_error, long long ops, double time_us) {
    std::cout << pad(method, 18) << pad(num(value, 8), 14) << pad(num(error_estimate, 2, true), 16)
              << pad(num(true_error, 2, true), 15) << pad(std::to_string(ops), 10)
              << num(time_us, 2) << '\n';
}

// Порівняння двох методів на основному наборі даних.
void compare_results(double value_a, double value_b, double true_err_a, double true_err_b,
                     long long ops_a, long long ops_b) {
    std::cout << "\nПорівняння:\n";
    std::cout << "  |y_Лагранж - y_сплайн| = " << num(std::abs(value_a - value_b), 2, true) << '\n';
    std::cout << "  Точніший метод (за фактичною похибкою): "
              << (true_err_a < true_err_b ? "Лагранж"
                  : true_err_b < true_err_a ? "кубічний сплайн" : "однаково") << '\n';
    std::cout << "  Менше арифметичних операцій: "
              << (ops_a < ops_b ? "Лагранж" : ops_b < ops_a ? "кубічний сплайн" : "однаково") << '\n';
}

// Як ростуть похибка та вартість зі збільшенням числа вузлів (функція Рунге).
void run_scaling_experiment() {
    const double x0 = 0.93;
    const double exact = f_runge(x0);
    constexpr int reps = 2000;  // усереднення часу за кількома запусками

    std::cout << "\nЕксперимент: f(x) = 1/(1+25x^2), [-1, 1], x0 = " << x0 << ", точне y = "
              << num(exact, 8) << "\n";
    std::cout << pad("n", 5) << pad("Похибка Лагр.", 15) << pad("Похибка спл.", 15)
              << pad("Опер. Лагр.", 13) << pad("Опер. спл.", 12) << pad("t Лагр., мкс", 14)
              << "t спл., мкс\n"
              << std::string(84, '-') << '\n';

    for (int n : {5, 9, 13, 17, 21, 41, 81}) {
        const auto data = make_input(f_runge, -1.0, 1.0, n, x0);  // один shared_ptr для обох методів
        double time_a = 0.0;
        double time_b = 0.0;
        std::unique_ptr<Result> ra;
        std::unique_ptr<Result> rb;
        for (int r = 0; r < reps; ++r) {
            ra = calculateA(data);
            rb = calculateB(data);
            time_a += ra->time_us;
            time_b += rb->time_us;
        }
        std::cout << pad(std::to_string(n), 5)
                  << pad(num(std::abs(ra->value - exact), 2, true), 15)
                  << pad(num(std::abs(rb->value - exact), 2, true), 15)
                  << pad(std::to_string(ra->operations), 13)
                  << pad(std::to_string(rb->operations), 12)
                  << pad(num(time_a / reps, 3), 14) << num(time_b / reps, 3) << '\n';
    }
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // щоб кирилиця коректно виводилась у консолі Windows
#endif

    try {
        // Єдиний спільний набір вхідних даних для обох алгоритмів.
        const double x0 = 1.1;
        const auto data = make_input(f_sin, 0.0, 3.0, 8, x0);
        const double exact = f_sin(x0);

        std::cout << "Інтерполяція таблично заданої функції y = sin(x)\n"
                  << "Вузлів: " << data->points.size() << " (рівномірно на [0, 3]), x0 = " << x0
                  << ", точне значення = " << num(exact, 8) << "\n\n";
        print_header();

        // ===== ДІЛЯНКА ВИКЛИКІВ АЛГОРИТМІВ =====
        // Сюди додають виклики своїх алгоритмів Студент А та Студент Б.
        auto resultA = calculateA(data);
        auto [methodA, valueA, errorA, opsA, timeA] = *resultA;
        print_row(methodA, valueA, errorA, std::abs(valueA - exact), opsA, timeA);
        auto resultB = calculateB(data);
        auto [methodB, valueB, errorB, opsB, timeB] = *resultB;
        print_row(methodB, valueB, errorB, std::abs(valueB - exact), opsB, timeB);
        // ===== КІНЕЦЬ ДІЛЯНКИ =====

        compare_results(valueA, valueB, std::abs(valueA - exact), std::abs(valueB - exact), opsA, opsB);
        run_scaling_experiment();
    } catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
