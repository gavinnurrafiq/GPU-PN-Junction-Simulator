# GPU PN Junction Simulator

A 1D semiconductor PN junction simulator written in **C++ and SYCL**, designed to execute numerical calculations on a GPU using the **Intel oneAPI DPC++/C++ Compiler (`icpx`)**.

The project solves a simplified semiconductor **Poisson equation** on a 1D spatial grid and calculates the resulting electric potential and electric field.

> **Status:** Educational / experimental numerical simulator
> **Language:** C++
> **GPU API:** SYCL
> **Compiler:** Intel oneAPI DPC++/C++ Compiler
> **Primary target:** Intel GPUs

---

## Features

* 1D abrupt PN junction
* Silicon material parameters
* Configurable temperature
* Configurable donor and acceptor doping
* 200,000 spatial grid points by default
* GPU-accelerated charge-density calculation
* GPU-accelerated Poisson iteration
* Electric-field calculation
* CSV output
* SYCL-based parallel execution
* Designed to run on Intel GPUs

The current implementation uses SYCL kernels such as `parallel_for()` to perform calculations over the spatial grid in parallel.

---

# 1. Requirements

## Hardware

A GPU capable of running the SYCL device runtime is recommended.

The primary target for this project is an **Intel GPU**, such as:

* Intel Arc B580
* Intel Arc A-series
* Other supported Intel GPU devices

A compatible and sufficiently recent Intel GPU driver is required when running SYCL workloads on Intel GPUs.

Intel's documentation recommends installing the latest Intel GPU driver when targeting an Intel GPU.

---

## Operating System

The current instructions are written for:

**Windows 10/11 64-bit**

The Intel oneAPI DPC++/C++ Compiler supports 64-bit Windows and Linux environments.

---

# 2. Required Software

## Intel oneAPI DPC++/C++ Compiler

This project requires the Intel oneAPI DPC++/C++ Compiler.

The compiler provides:

```text
icpx
SYCL
DPC++
Intel SYCL runtime
```

The official Intel compiler download page is:

[Intel oneAPI DPC++/C++ Compiler](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler-download.html?utm_source=chatgpt.com)

The current Intel compiler release page lists version 2026.1.1.

You can install the standalone DPC++/C++ Compiler or install the Intel oneAPI Toolkit.

---

# 3. Microsoft Visual Studio

On Windows, install Visual Studio with:

```text
Desktop development with C++
```

Microsoft C++ support is required for full functionality with the Intel compiler and Visual Studio integration. Intel's Windows setup documentation lists Visual Studio 2022 and 2019 among supported versions in its corresponding guide.

Visual Studio is not necessarily required as the editor for this project, but its C++ development components are important for the Windows compiler environment.

---

# 4. Intel GPU Driver

If the program is intended to execute on an Intel GPU, install the appropriate Intel graphics driver.

Verify that Windows recognizes the GPU before running the simulator.

For an Intel Arc GPU, for example:

```text
Intel Arc B580
```

should appear correctly in Windows Device Manager.

---

# 5. Initializing the oneAPI Environment

The `icpx` command may not be available from a normal PowerShell or Command Prompt until the oneAPI environment has been initialized.

Intel provides `setvars.bat` for configuring the required environment variables.

The default installation location is typically:

```text
C:\Program Files (x86)\Intel\oneAPI\
```

The environment script is:

```text
C:\Program Files (x86)\Intel\oneAPI\setvars.bat
```

---

# 6. Recommended Method: Intel oneAPI Command Prompt

The easiest method on Windows is to open:

```text
Intel oneAPI Command Prompt
```

from the Windows Start Menu.

Then verify the compiler:

```cmd
icpx --version
```

A successful installation should display information about the Intel oneAPI DPC++/C++ Compiler.

Intel's Windows documentation notes that the oneAPI compiler command-line environment normally initializes the required environment variables automatically.

---

# 7. Manual Environment Initialization

If the Intel oneAPI Command Prompt is not available, initialize the environment manually.

Open Command Prompt and run:

```cmd
"C:\Program Files (x86)\Intel\oneAPI\setvars.bat"
```

Then verify:

```cmd
icpx --version
```

You can also check:

```cmd
set SETVARS_COMPLETED
```

The expected result is:

```text
SETVARS_COMPLETED=1
```

The `setvars.bat` script initializes the environment variables for the installed oneAPI components.

---

# 8. PowerShell

If you normally use PowerShell, you can launch a PowerShell environment from the initialized oneAPI environment.

For example:

```cmd
cmd.exe /K ""C:\Program Files (x86)\Intel\oneAPI\setvars.bat" && powershell"
```

Then verify:

```powershell
icpx --version
```

---

# 9. Verify SYCL Compiler

Run:

```powershell
icpx --version
```

Then verify the compiler can process SYCL.

Create a simple test file:

```cpp
#include <sycl/sycl.hpp>
#include <iostream>

int main()
{
    sycl::queue q{sycl::gpu_selector_v};

    std::cout
        << "GPU: "
        << q.get_device()
             .get_info<sycl::info::device::name>()
        << "\n";

    return 0;
}
```

Save it as:

```text
test_sycl.cpp
```

Compile:

```powershell
icpx -fsycl test_sycl.cpp -o test_sycl.exe
```

Run:

```powershell
.\test_sycl.exe
```

If everything is configured correctly, the program should print the selected GPU.

Intel documents `-fsycl` as the option that enables SYCL compilation.

---

# 10. Compile the PN Junction Simulator

Navigate to the directory containing the source code.

For example:

```powershell
cd C:\Users\Admin\Documents
```

Assuming the source file is:

```text
pnjunction.cpp
```

compile with:

```powershell
icpx -fsycl pnjunction.cpp -o pnjunction.exe
```

The important option is:

```text
-fsycl
```

Without it, the source is not compiled as a SYCL application.

Intel documents the same compilation pattern:

```text
icpx -fsycl hello-world.cpp
```

for SYCL C++ applications.

---

# 11. Run the Simulator

After successful compilation:

```powershell
.\pnjunction.exe
```

Expected output will look approximately like:

```text
========================================
      GPU PN Junction Simulator
========================================

Grid points : 200000
Device size : 2 um
Temperature : 300 K
Na          : 1e+17 cm^-3
Nd          : 1e+17 cm^-3
dx          : ...

GPU: Intel(R) Arc(TM) B580 Graphics

Starting GPU Poisson solver...
...
Simulation finished.
Results saved to pn_junction.csv
========================================
```

The exact output depends on the GPU, compiler version, runtime, and solver behavior.

---

# 12. Output

The simulator generates:

```text
pn_junction.csv
```

The CSV contains:

```text
x_m
potential_V
electric_field_V_per_m
```

Example:

```csv
x_m,potential_V,electric_field_V_per_m
0.0,...
1.0e-11,...
2.0e-11,...
...
```

The data can be opened using:

* Microsoft Excel
* LibreOffice Calc
* Python
* MATLAB
* GNU Octave
* Origin
* other numerical-analysis software

---

# 13. Physical Model

The simulator represents an abrupt 1D PN junction.

The charge density is modeled as:

```text
rho = q(p - n + Nd - Na)
```

where:

```text
q  = elementary charge
p  = hole concentration
n  = electron concentration
Nd = donor concentration
Na = acceptor concentration
```

The Poisson equation is:

```text
d²V/dx² = -rho/epsilon
```

The electric field is obtained from:

```text
E = -dV/dx
```

The current implementation uses Boltzmann-like carrier expressions for `n` and `p`.

---

# 14. Default Parameters

The current source uses approximately:

```text
Temperature:
300 K

Silicon relative permittivity:
11.7

Intrinsic carrier concentration:
1.0 × 10^10 cm^-3

Acceptor concentration:
1.0 × 10^17 cm^-3

Donor concentration:
1.0 × 10^17 cm^-3

Simulation length:
2 µm

Grid points:
200,000
```

The doping profile is an abrupt junction:

```text
P-type | N-type
-------|-------
 Na    |  Nd
```

with the junction positioned approximately at the center of the simulation domain.

---

# 15. Why GPU?

The simulation divides the semiconductor into many spatial points.

For example:

```text
N = 200,000
```

means that the device is represented by 200,000 grid points.

Many calculations for these points are independent and therefore suitable for parallel execution.

SYCL allows these calculations to be expressed using kernels such as:

```cpp
queue.submit([&](sycl::handler& h)
{
    h.parallel_for(
        sycl::range<1>(N),
        [=](sycl::id<1> i)
        {
            // Calculation for one grid point
        }
    );
});
```

Conceptually:

```text
CPU
 │
 └── Submit work
        │
        ▼
      GPU
 ┌────┬────┬────┬────┬────┐
 │ x0 │ x1 │ x2 │ x3 │ ...│
 └────┴────┴────┴────┴────┘
        │
        ▼
 Parallel numerical calculation
```

This project is therefore also an experiment in **GPU-accelerated computational physics**.

---

# 16. Project Structure

A minimal version of the project can look like:

```text
PNJunction-GPU/
│
├── pnjunction.cpp
├── README.md
│
└── pn_junction.csv
```

The CSV file is generated after running the program and does not have to be committed to GitHub unless desired.

A more advanced version may eventually use:

```text
PNJunction-GPU/
│
├── include/
│   ├── PNJunction.hpp
│   └── GPUSolver.hpp
│
├── src/
│   ├── PNJunction.cpp
│   ├── GPUSolver.cpp
│   └── main.cpp
│
├── results/
│
├── README.md
└── CMakeLists.txt
```

---

# 17. Troubleshooting

## `sycl/sycl.hpp: No such file or directory`

Example:

```text
fatal error: sycl/sycl.hpp: No such file or directory
```

This usually means the source is being compiled with a compiler/environment that cannot locate the SYCL headers.

Do not compile with:

```text
g++
```

for this project.

Use:

```text
icpx
```

and:

```text
-fsycl
```

Example:

```powershell
icpx -fsycl pnjunction.cpp -o pnjunction.exe
```

Also make sure the oneAPI environment has been initialized.

---

## `icpx is not recognized`

Example:

```text
'icpx' is not recognized...
```

Initialize oneAPI:

```cmd
"C:\Program Files (x86)\Intel\oneAPI\setvars.bat"
```

or open the Intel oneAPI Command Prompt.

Then:

```cmd
icpx --version
```

---

## GPU is not detected

If the application fails to find a GPU, check:

1. Intel GPU driver
2. Intel GPU visibility in Windows
3. oneAPI installation
4. SYCL runtime
5. selected SYCL device

A simple test:

```cpp
sycl::queue q{sycl::gpu_selector_v};

std::cout
    << q.get_device()
       .get_info<sycl::info::device::name>();
```

If the GPU selector cannot find an appropriate device, the issue is likely environmental rather than related to the PN junction equations.

---

# 18. Important Numerical / Physical Limitations

This project is intended primarily for **education, experimentation, and GPU programming research**.

The current implementation is **not a production TCAD simulator**.

Important limitations include:

* simplified carrier statistics
* simplified equilibrium assumptions
* simplified boundary conditions
* simplified doping model
* no advanced mobility model
* no detailed recombination model
* no generation model
* no Fermi-Dirac statistics
* no quantum effects
* no temperature-dependent material model
* 1D geometry only
* simplified numerical solver
* no rigorous validation against commercial TCAD software

Therefore, numerical results should not be interpreted as experimentally validated semiconductor-device predictions.

---

# 19. Theoretical Validation

For an equilibrium PN junction, the simulator can eventually be compared against the analytical built-in potential:

```text
Vbi = VT ln(Na Nd / ni²)
```

where:

```text
VT = kT/q
```

For the default parameters:

```text
Na = 1 × 10^17 cm^-3
Nd = 1 × 10^17 cm^-3
ni = 1 × 10^10 cm^-3
T  = 300 K
```

the analytical result can be used as a reference for validating the numerical solver.

Future versions should compare:

```text
Analytical solution
        vs
CPU numerical solution
        vs
GPU numerical solution
```

---

# 20. Future Development

Possible extensions include:

### Semiconductor physics

* Depletion approximation
* Built-in potential
* Debye length
* depletion width
* carrier concentration
* Fermi level
* band diagrams
* Shockley diode equation
* drift-diffusion
* continuity equations
* recombination
* generation
* temperature dependence

### Device simulation

```text
PN Junction
     ↓
Diode
     ↓
MOS Capacitor
     ↓
MOSFET
     ↓
BJT
```

### GPU computing

* optimized SYCL kernels
* GPU/CPU benchmarking
* Conjugate Gradient solver
* multigrid methods
* 2D simulation
* 3D simulation
* memory optimization
* mixed precision
* GPU profiling

### Visualization

The CSV output can eventually be connected to a visualization interface:

```text
Potential V(x)
Electric field E(x)
Electron density n(x)
Hole density p(x)
Charge density rho(x)
```

---

# 21. Example Workflow

Complete workflow on Windows:

```cmd
:: 1. Open Intel oneAPI Command Prompt

:: 2. Check compiler
icpx --version

:: 3. Enter project directory
cd /d C:\Users\Admin\Documents

:: 4. Compile
icpx -fsycl pnjunction.cpp -o pnjunction.exe

:: 5. Run
pnjunction.exe
```

PowerShell:

```powershell
cd C:\Users\Admin\Documents

icpx -fsycl pnjunction.cpp -o pnjunction.exe

.\pnjunction.exe
```

---

# 22. License

Add the project's license here.

For example:

```text
MIT License
```

if the project is intended to use the MIT License.

---

# 23. References

* Intel oneAPI DPC++/C++ Compiler
  [Official Intel DPC++/C++ Compiler page](https://www.intel.com/content/www/us/en/developer/tools/oneapi/dpc-compiler-download.html?utm_source=chatgpt.com)

* Intel oneAPI DPC++/C++ Compiler — Get Started on Windows
  [Intel Windows Get Started Guide](https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/get-started-guide/2025-2/get-started-on-windows.html?utm_source=chatgpt.com)

* Intel oneAPI Compiler Documentation
  [Intel DPC++/C++ Compiler Developer Guide and Reference](https://www.intel.com/content/www/us/en/docs/dpcpp-cpp-compiler/developer-guide-reference/2026-0/use-the-intel-oneapi-dpc-c-compiler.html?utm_source=chatgpt.com)

* SYCL Specification
  [Khronos SYCL Reference](https://registry.khronos.org/SYCL/?utm_source=chatgpt.com)

---

## Disclaimer

This project is an educational computational-physics project. It is intended to demonstrate numerical semiconductor physics and GPU programming using C++ and SYCL. It should not be used as a substitute for validated semiconductor-device simulation software or experimental measurements.
