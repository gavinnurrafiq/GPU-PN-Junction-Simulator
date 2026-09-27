#include <sycl/sycl.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace sycl;

// ============================================================
// Physical constants
// ============================================================

constexpr double q  = 1.602176634e-19;   // Elementary charge [C]
constexpr double kB = 1.380649e-23;      // Boltzmann constant [J/K]
constexpr double eps0 = 8.8541878128e-12;

constexpr double T = 300.0;              // Temperature [K]

// Silicon relative permittivity
constexpr double eps_si = 11.7;
constexpr double epsilon = eps0 * eps_si;

// Intrinsic carrier concentration of silicon at 300 K
// Approximate value: 1.0e10 cm^-3
constexpr double ni_cm3 = 1.0e10;

// Convert cm^-3 -> m^-3
constexpr double ni = ni_cm3 * 1.0e6;

// ============================================================
// PN junction parameters
// ============================================================

// Doping concentrations
constexpr double Na_cm3 = 1.0e17;
constexpr double Nd_cm3 = 1.0e17;

constexpr double Na = Na_cm3 * 1.0e6;
constexpr double Nd = Nd_cm3 * 1.0e6;

// Thermal voltage
constexpr double Vt = kB * T / q;

// ============================================================
// Simulation domain
// ============================================================

constexpr std::size_t N = 200000;

// 2 micrometers total device length
constexpr double L = 2.0e-6;

constexpr double dx = L / static_cast<double>(N - 1);

// Maximum Poisson iterations
constexpr int MAX_ITER = 100000;

// Convergence criterion
constexpr double TOL = 1e-8;

// ============================================================
// Charge density
// ============================================================

double calculate_rho(
    double V,
    double Na_local,
    double Nd_local
)
{
    // Boltzmann carrier concentrations

    double exponent_n = V / Vt;
    double exponent_p = -V / Vt;

    // Prevent numerical overflow
    exponent_n = std::clamp(exponent_n, -50.0, 50.0);
    exponent_p = std::clamp(exponent_p, -50.0, 50.0);

    double n = ni * std::exp(exponent_n);
    double p = ni * std::exp(exponent_p);

    // rho = q(p - n + Nd - Na)

    return q * (
        p
        - n
        + Nd_local
        - Na_local
    );
}

// ============================================================
// Main
// ============================================================

int main()
{
    std::cout << "========================================\n";
    std::cout << "      GPU PN Junction Simulator\n";
    std::cout << "========================================\n\n";

    std::cout << "Grid points : " << N << "\n";
    std::cout << "Device size : " << L * 1e6 << " um\n";
    std::cout << "Temperature : " << T << " K\n";
    std::cout << "Na          : " << Na_cm3 << " cm^-3\n";
    std::cout << "Nd          : " << Nd_cm3 << " cm^-3\n";
    std::cout << "dx          : " << dx * 1e9 << " nm\n\n";

    // ========================================================
    // Select GPU
    // ========================================================

    queue gpuQueue(
        gpu_selector_v,
        [](exception_list exceptions)
        {
            for (auto& e : exceptions)
            {
                try
                {
                    std::rethrow_exception(e);
                }
                catch (const sycl::exception& ex)
                {
                    std::cerr
                        << "ASYNC SYCL ERROR: "
                        << ex.what()
                        << "\n";
                }
            }
        }
    );

    auto device = gpuQueue.get_device();

    std::cout
        << "GPU: "
        << device.get_info<info::device::name>()
        << "\n\n";

    // ========================================================
    // Host arrays
    // ========================================================

    std::vector<double> V(N, 0.0);
    std::vector<double> V_new(N, 0.0);

    std::vector<double> rho(N, 0.0);
    std::vector<double> E(N, 0.0);

    std::vector<double> Na_array(N);
    std::vector<double> Nd_array(N);

    // ========================================================
    // Create abrupt PN junction
    //
    // x < 0 : P-type
    // x > 0 : N-type
    // ========================================================

    const double junction = L / 2.0;

    for (std::size_t i = 0; i < N; ++i)
    {
        double x =
            static_cast<double>(i) * dx;

        if (x < junction)
        {
            Na_array[i] = Na;
            Nd_array[i] = 0.0;
        }
        else
        {
            Na_array[i] = 0.0;
            Nd_array[i] = Nd;
        }
    }

    // ========================================================
    // Device buffers
    // ========================================================

    buffer<double, 1> V_buf(
        V.data(),
        range<1>(N)
    );

    buffer<double, 1> Vnew_buf(
        V_new.data(),
        range<1>(N)
    );

    buffer<double, 1> rho_buf(
        rho.data(),
        range<1>(N)
    );

    buffer<double, 1> Na_buf(
        Na_array.data(),
        range<1>(N)
    );

    buffer<double, 1> Nd_buf(
        Nd_array.data(),
        range<1>(N)
    );

    // ========================================================
    // Poisson solver
    // ========================================================

    std::cout << "Starting GPU Poisson solver...\n";

    for (int iteration = 0;
         iteration < MAX_ITER;
         ++iteration)
    {
        // ----------------------------------------------------
        // Calculate charge density
        // ----------------------------------------------------

        gpuQueue.submit(
            [&](handler& h)
            {
                auto V_acc =
                    V_buf.get_access<access::mode::read>(h);

                auto rho_acc =
                    rho_buf.get_access<access::mode::write>(h);

                auto Na_acc =
                    Na_buf.get_access<access::mode::read>(h);

                auto Nd_acc =
                    Nd_buf.get_access<access::mode::read>(h);

                h.parallel_for(
                    range<1>(N),
                    [=](id<1> idx)
                    {
                        std::size_t i = idx[0];

                        double voltage = V_acc[i];

                        double en =
                            sycl::exp(
                                sycl::clamp(
                                    voltage / Vt,
                                    -50.0,
                                    50.0
                                )
                            );

                        double ep =
                            sycl::exp(
                                sycl::clamp(
                                    -voltage / Vt,
                                    -50.0,
                                    50.0
                                )
                            );

                        double n = ni * en;
                        double p = ni * ep;

                        rho_acc[i] =
                            q *
                            (
                                p
                                - n
                                + Nd_acc[i]
                                - Na_acc[i]
                            );
                    }
                );
            }
        );

        // ----------------------------------------------------
        // Jacobi iteration
        // ----------------------------------------------------

        gpuQueue.submit(
            [&](handler& h)
            {
                auto V_acc =
                    V_buf.get_access<access::mode::read>(h);

                auto Vnew_acc =
                    Vnew_buf.get_access<access::mode::write>(h);

                auto rho_acc =
                    rho_buf.get_access<access::mode::read>(h);

                h.parallel_for(
                    range<1>(N - 2),
                    [=](id<1> idx)
                    {
                        std::size_t i =
                            idx[0] + 1;

                        Vnew_acc[i] =
                            0.5 *
                            (
                                V_acc[i - 1]
                                +
                                V_acc[i + 1]
                                +
                                rho_acc[i]
                                * dx
                                * dx
                                / epsilon
                            );
                    }
                );
            }
        );

        // Boundary conditions
        V_new[0] = 0.0;
        V_new[N - 1] = 0.0;

        // Synchronize and obtain result
        gpuQueue.wait();

        // Copy V_new back to V
        {
            sycl::host_accessor Vnew_host(
                Vnew_buf,
                sycl::read_only
            );

            for (std::size_t i = 1;
                 i < N - 1;
                 ++i)
            {
                V[i] = Vnew_host[i];
            }
        }

        // ----------------------------------------------------
        // Check convergence
        // ----------------------------------------------------

        double maxError = 0.0;

        for (std::size_t i = 1;
             i < N - 1;
             ++i)
        {
            double error =
                std::abs(V_new[i] - V[i]);

            maxError =
                std::max(maxError, error);
        }

        if (iteration % 100 == 0)
        {
            std::cout
                << "Iteration "
                << iteration
                << "   error = "
                << maxError
                << "\n";
        }

        if (maxError < TOL)
        {
            std::cout
                << "\nConverged after "
                << iteration
                << " iterations.\n";

            break;
        }
    }

    // ========================================================
    // Calculate electric field
    //
    // E = -dV/dx
    // ========================================================

    gpuQueue.submit(
        [&](handler& h)
        {
            auto V_acc =
                V_buf.get_access<access::mode::read>(h);

            auto rho_acc =
                rho_buf.get_access<access::mode::read>(h);

            auto E_buf =
                buffer<double, 1>(
                    E.data(),
                    range<1>(N)
                );

            auto E_acc =
                E_buf.get_access<access::mode::write>(h);

            h.parallel_for(
                range<1>(N - 2),
                [=](id<1> idx)
                {
                    std::size_t i =
                        idx[0] + 1;

                    E_acc[i] =
                        -(
                            V_acc[i + 1]
                            -
                            V_acc[i - 1]
                        )
                        /
                        (2.0 * dx);
                }
            );
        }
    );

    gpuQueue.wait();

    // ========================================================
    // Export results
    // ========================================================

    std::ofstream file(
        "pn_junction.csv"
    );

    file
        << "x_m,potential_V,electric_field_V_per_m\n";

    auto E_final =
        buffer<double, 1>(
            E.data(),
            range<1>(N)
        );

    {
        sycl::host_accessor E_host(
            E_final,
            sycl::read_only
        );

        for (std::size_t i = 0;
             i < N;
             ++i)
        {
            double x =
                static_cast<double>(i) * dx;

            file
                << x
                << ","
                << V[i]
                << ","
                << E_host[i]
                << "\n";
        }
    }

    file.close();

    // ========================================================
    // Basic result
    // ========================================================

    auto minmaxV =
        std::minmax_element(
            V.begin(),
            V.end()
        );

    std::cout << "\n========================================\n";
    std::cout << "Simulation finished.\n";
    std::cout << "Minimum potential: "
              << *minmaxV.first
              << " V\n";

    std::cout << "Maximum potential: "
              << *minmaxV.second
              << " V\n";

    std::cout
        << "Results saved to pn_junction.csv\n";

    std::cout << "========================================\n";

    return 0;
}