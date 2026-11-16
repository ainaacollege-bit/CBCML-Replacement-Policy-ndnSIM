# CBCML-Replacement-Policy-ndnSIM
Random Forest-Enhanced Multi-Factor Cache Replacement Strategy for Information-Centric Networking

## Project Overview

This repository provides the implementation of the **Random Forest-Enhanced Multi-Factor Cache (CBCML) replacement strategy** for **Information-Centric Networking (ICN)** using the **ndnSIM 2.9 simulator**.

CBCML extends the Cost-Based Cache (CBC) replacement strategy by incorporating **Machine Learning (ML)-based content popularity prediction** using **Random Forest Regression (RFR)**. The proposed approach combines multiple caching factors with predicted content popularity to make more informed cache replacement decisions.

Unlike conventional caching strategies such as **FIFO, LRU, and LRFU**, CBCML integrates historical content access information and machine learning-based prediction to improve cache management and content delivery performance.

The proposed CBCML strategy is evaluated through simulation experiments using different network topologies, cache sizes, and Zipf distribution parameters.

## Simulator Environment

The simulation experiments were conducted using **ndnSIM 2.9**, an open-source network simulator built on **NS-3** and designed for the **Named Data Networking (NDN)** architecture.

### System Specifications

- **Processor:** Intel Core i5-9400F CPU @ 2.90 GHz
- **Memory:** 8 GB RAM
- **Operating System:** Ubuntu 20.04 LTS
- **Network Simulator:** ndnSIM 2.9 (NS-3 based)
- **Machine Learning Algorithm:** Random Forest Regression (RFR)
- **Machine Learning Library:** Scikit-learn

## Machine Learning Configuration

CBCML incorporates Random Forest Regression to predict content popularity based on historical content access patterns.

### Random Forest Regression Parameters

The following parameters are used in the implementation:

| Parameter | Value |
|---|---|
| Algorithm | Random Forest Regression |
| Number of Estimators | 100 |
| Random State | 42 |

The Random Forest model is used to estimate future content popularity, which contributes to the cache replacement decision-making process.

## Network Topology Scenarios

Two network topology scenarios are used to evaluate the effectiveness of the proposed CBCML replacement strategy.

### Scenario 1: 5×5 Grid Topology

This topology represents a structured and controlled network environment.

- **Total nodes:** 25
- **Topology:** 5×5 Grid
- **Content producers:** 2 nodes (Node 0 and Node 20)
- **Consumer nodes:** 5 nodes
- **Remaining nodes:** Routers with caching capability

This scenario enables controlled experimentation to investigate caching behaviour and content dissemination patterns.

### Scenario 2: Abilene Topology

The Abilene topology represents a real-world backbone network structure.

- **Total nodes:** 11
- **Topology:** Abilene Internet2 network
- **Network nodes:** Major routers from the original Abilene topology
- **Node functionality:** Each node functions as both producer and consumer

This scenario evaluates CBCML under a more realistic and heterogeneous network environment.

## Cache Replacement Strategies

The proposed CBCML strategy is evaluated against several existing cache replacement strategies commonly used in ICN research.

### Compared Strategies

- **FIFO** – First In First Out
- **LRU** – Least Recently Used
- **LRFU** – Least Recently/Frequently Used
- **PFR** – Popularity Freshness Replacement
- **CBC** – Cost-Based Cache
- **CBCML** – Random Forest-Enhanced Multi-Factor Cache Replacement

## Performance Metrics

The performance evaluation focuses on three key metrics.

### Cache Hit Ratio (CHR)

Measures the proportion of content requests successfully served from the cache. A higher CHR indicates more efficient cache utilization.

### Cache Miss Ratio (CMR)

Represents the proportion of content requests that cannot be satisfied by the cache and must be forwarded towards the content producer. A lower CMR indicates better caching performance.

### Average Retrieval Delay

Measures the average time required to deliver requested content to consumers. Lower retrieval delay indicates faster content delivery.

## Repository Structure

The repository is organized into the following directories:

```text
CBCML-Random-Forest-ndnSIM/
│
├── examples/
│   └── Simulation scenario scripts and topology configurations
│
├── helper/
│   └── Helper modules and utility functions
│
├── table/
│   └── CBCML cache replacement implementation
│
├── ml/
│   └── Random Forest Regression implementation
│
├── results/
│   └── Experimental results and simulation output
│
├── README.md
└── requirements.txt
```

**Note:** The directory structure above represents the recommended organization. Actual paths may differ depending on the implementation.

## Installation and Setup

### 1. Install ndnSIM 2.9

First, install the required simulation environment.

Follow the official ndnSIM installation guide:

https://ndnsim.net/current/getting-started.html

Ensure the following components are installed:

- Ubuntu 20.04 LTS (recommended)
- NS-3
- ndnSIM 2.9
- Required C++ development tools

### 2. Install Python and Machine Learning Dependencies

CBCML uses Python and Scikit-learn for the Random Forest Regression implementation.

Install Python 3 and pip if they are not already available:

```bash
sudo apt update
sudo apt install python3 python3-pip python3-venv
```

Create a Python virtual environment:

```bash
python3 -m venv cbcml-env
source cbcml-env/bin/activate
```

Install the required Python libraries:

```bash
pip install numpy pandas scikit-learn matplotlib
```

Alternatively, install the dependencies using the provided requirements file:

```bash
pip install -r requirements.txt
```

### 3. Clone the CBCML Repository

Clone the project repository:

```bash
git clone https://github.com/ainaacollege-bit/CBCML-Random-Forest-ndnSIM.git
```

Navigate to the repository directory:

```bash
cd CBCML-Random-Forest-ndnSIM
```

### 4. Integrate CBCML with ndnSIM

Ensure that the CBCML source files are placed in their corresponding ndnSIM directories.

The implementation includes:

- CBCML cache replacement policy.
- Random Forest Regression-based popularity prediction.
- Supporting helper functions and data structures.
- Simulation scenario scripts.

Refer to the repository structure and source files for the implementation details.

### 5. Compile the Simulator

Navigate to the ndnSIM directory and configure the simulation environment:

```bash
./waf configure
```

Compile the simulator:

```bash
./waf
```

Ensure that the compilation completes successfully before executing the simulation scenarios.

### 6. Run the Simulation

The simulation scenario scripts are located in the `examples/` directory.

Run the appropriate simulation script using:

```bash
./waf --run examples/<simulation-file>
```

Replace `<simulation-file>` with the corresponding simulation scenario filename.

Configure the simulation parameters according to the experimental requirements, including:

- Network topology.
- Content store (CS) size.
- Number of content items.
- Request rate.
- Zipf distribution parameter.

### 7. Execute the Random Forest Prediction

The Random Forest Regression implementation is provided in the `ml/` directory.

Run the appropriate Python script using:

```bash
python3 ml/<prediction-script>.py
```

Replace `<prediction-script>` with the actual Python script filename.

Ensure that the required input data and configuration files are available before executing the prediction process.

## Research Contribution

This project contributes to the research area of **cache management in Information-Centric Networking (ICN)** by introducing a machine learning-enhanced multi-factor cache replacement strategy.

The proposed CBCML approach integrates:

- Multi-factor cost-based cache replacement.
- Random Forest Regression for content popularity prediction.
- Historical content access information.
- Machine learning-enhanced cache replacement decisions.

The objective is to improve:

- Cache hit ratio.
- Cache miss ratio.
- Average content retrieval delay.
- Overall cache utilization and network performance.

## License

This project is intended for academic and research purposes.

Please refer to the repository license and the licensing terms of the underlying ndnSIM and NS-3 components before reusing or redistributing the implementation.

## Citation

If you use this implementation in your research, please cite the associated research publication.

**Zenodo DOI:** To be added after publication.

**GitHub Repository:**  
https://github.com/ainaacollege-bit/CBCML-Random-Forest-ndnSIM
