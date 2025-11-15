#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <execution>
#include <algorithm>
#include <iterator>

using namespace std;

// комментарий
// Шаблонная функция для заполнения контейнера случайными числами
template<typename Container>
void fillContainer(Container& cont, size_t count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(-100, 100);
    auto randval = [&gen, &dis]() { return dis(gen); };

    for (size_t i = 1; i < count; ++i) {
        cont.insert(cont.end(), randval());
    }
}

// Шаблонная функция для вывода содержимого контейнера
template<typename Container>
void printContainer(const Container& cont, const std::string& name) {
    std::cout << name << ": ";
    std::copy(cont.begin(), cont.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;
}

// Функция для формирования коллекции L (элементы, входящие только в одну из коллекций)
template<typename Container>
Container SymmetricDifferenceNoPolicy(const Container& l1, const Container& l2) {
    Container temp1 = l1;
    Container temp2 = l2;

    // Сортировка контейнеров
    std::sort(temp1.begin(), temp1.end());
    std::sort(temp2.begin(), temp2.end());

    Container result;

    // Используем алгоритм set_symmetric_difference для нахождения симметрической разности
    std::set_symmetric_difference(
        temp1.begin(), temp1.end(),
        temp2.begin(), temp2.end(),
        std::back_inserter(result)
    );

    // Удаление дубликатов (если были в исходных коллекциях)
    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());

    return result;
}

// Функция с применением политики распараллеливания seq
template<typename Container, typename ExecutionPolicy = std::execution::sequenced_policy>
Container SymmetricDifferencePolicySeq(const Container& l1, const Container& l2,
    ExecutionPolicy policy = std::execution::seq) {
    Container temp1 = l1;
    Container temp2 = l2;

    // Сортировка контейнеров с политикой
    std::sort(policy, temp1.begin(), temp1.end());
    std::sort(policy, temp2.begin(), temp2.end());

    Container result;

    std::set_symmetric_difference(
        temp1.begin(), temp1.end(),
        temp2.begin(), temp2.end(),
        std::back_inserter(result)
    );

    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());

    return result;
}

// Функция с применением политики распараллеливания par
template<typename Container, typename ExecutionPolicy = std::execution::parallel_policy>
Container SymmetricDifferencePolicyPar(const Container& l1, const Container& l2,
    ExecutionPolicy policy = std::execution::par) {
    Container temp1 = l1;
    Container temp2 = l2;

    std::sort(policy, temp1.begin(), temp1.end());
    std::sort(policy, temp2.begin(), temp2.end());

    Container result;

    std::set_symmetric_difference(
        temp1.begin(), temp1.end(),
        temp2.begin(), temp2.end(),
        std::back_inserter(result)
    );

    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());

    return result;
}
// Функция с применением политики распараллеливания par_unseq
template<typename Container, typename ExecutionPolicy = std::execution::parallel_unsequenced_policy>
Container SymmetricDifferencePolicyPUN(const Container& l1, const Container& l2,
    ExecutionPolicy policy = std::execution::par_unseq) {
    Container temp1 = l1;
    Container temp2 = l2;

    std::sort(policy, temp1.begin(), temp1.end());
    std::sort(policy, temp2.begin(), temp2.end());

    Container result;

    std::set_symmetric_difference(
        temp1.begin(), temp1.end(),
        temp2.begin(), temp2.end(),
        std::back_inserter(result)
    );

    auto last = std::unique(result.begin(), result.end());
    result.erase(last, result.end());

    return result;
}

int main() {
    // Вывод количества потоков
    unsigned int available_threads = std::thread::hardware_concurrency();
    std::cout << "Number of streams at the same time: " << available_threads << std::endl;

    // Демонстрация с малым количеством элементов
    std::cout << "Demonstration with small collections" << std::endl;

    std::vector<int> v1 = { 1, 2, 3, 4, 5, 2, 1 };
    std::vector<int> v2 = { 4, 5, 6, 7, 8, 6, 7 };
    std::vector<int> vres = SymmetricDifferenceNoPolicy(v1, v2);

    printContainer(v1, "Vector 1");
    printContainer(v2, "Vector 2");
    printContainer(vres, "Symmetric difference result");
    std::cout << std::endl;

    // Демонстрация с большим количеством элементов
    std::cout << "Demonstration with big collections" << std::endl;
    const size_t size = 10'000'000;

    std::vector<int> bigv1, bigv2;
    fillContainer(bigv1, size);
    fillContainer(bigv2, size);

    // Замер времени без политики
    auto start = std::chrono::high_resolution_clock::now();
    auto bigvres = SymmetricDifferenceNoPolicy(bigv1, bigv2);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "No policy time: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
        << " ms" << std::endl;

    // Замер времени с политикой seq
    auto startseq = std::chrono::high_resolution_clock::now();
    auto bigvresseq = SymmetricDifferencePolicySeq(bigv1, bigv2);
    auto endseq = std::chrono::high_resolution_clock::now();
    std::cout << "Seq time: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(endseq - startseq).count()
        << " ms" << std::endl;

    // Замер времени с политикой par
    auto startpar = std::chrono::high_resolution_clock::now();
    auto bigvrespar = SymmetricDifferencePolicyPar(bigv1, bigv2);
    auto endpar = std::chrono::high_resolution_clock::now();
    std::cout << "Par time: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(endpar - startpar).count()
        << " ms" << std::endl;

    // Замер времени с политикой par_unseq
    auto startpun = std::chrono::high_resolution_clock::now();
    auto bigvrespun = SymmetricDifferencePolicyPUN(bigv1, bigv2);
    auto endpun = std::chrono::high_resolution_clock::now();
    std::cout << "Par_unseq time: "
        << std::chrono::duration_cast<std::chrono::milliseconds>(endpun - startpun).count()
        << " ms" << std::endl;

    return 0;
}
