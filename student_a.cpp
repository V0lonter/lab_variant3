#include "shared_types.h"

// TODO (Студент А): інтерполяція Лагранжа
std::unique_ptr<Result> calculateA(std::shared_ptr<const InputData> data) {
    (void)data;
    return std::make_unique<Result>(Result{"Lagrange (stub)", 0.0, 0.0, 0, 0.0});
}
