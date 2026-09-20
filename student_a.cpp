// Студент А — інтерполяція Лагранжа.
#include <chrono>

#include "shared_types.h"

namespace {

// P(x) = sum_i y_i * prod_{j != i} (x - x_j) / (x_i - x_j)
// ops — лічильник арифметичних операцій (порівняння та індексація не рахуються).
double lagrange_value(const std::vector<Point>& p, double x, long long& ops) {
    double sum = 0.0;
    for (std::size_t i = 0; i < p.size(); ++i) {
        double term = p[i].y;
        for (std::size_t j = 0; j < p.size(); ++j) {
            if (j == i) {
                continue;
            }
            term *= (x - p[j].x) / (p[i].x - p[j].x);
            ops += 4;  // 2 віднімання + 1 ділення + 1 множення
        }
        sum += term;
        ops += 1;  // додавання
    }
    return sum;
}

}  // namespace

std::unique_ptr<Result> calculateA(std::shared_ptr<const InputData> data) {
    validate_input(data);
    const std::vector<Point>& pts = data->points;
    const double x0 = data->x0;

    // --- основне обчислення (міряємо час і рахуємо операції) ---
    long long ops = 0;
    const auto t0 = std::chrono::steady_clock::now();
    const double value = lagrange_value(pts, x0, ops);
    const auto t1 = std::chrono::steady_clock::now();
    const double time_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    // --- оцінка похибки ---
    // Будуємо поліном без вузла, найближчого до x0, і беремо |P_n(x0) - P_{n-1}(x0)|.
    // Це евристична оцінка: вона показує, наскільки результат чутливий до складу вузлів.
    std::vector<Point> reduced = pts;
    reduced.erase(reduced.begin() + static_cast<std::ptrdiff_t>(nearest_interior_index(pts, x0)));
    long long unused_ops = 0;  // вартість оцінки в загальний підрахунок не входить
    const double error_estimate = std::abs(value - lagrange_value(reduced, x0, unused_ops));

    return std::make_unique<Result>(Result{"Лагранж", value, error_estimate, ops, time_us});
}
