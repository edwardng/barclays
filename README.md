# Compiling and Building
1. create a build directory such as ~/build and `cd` into this directory
2. cmake `target_directory` where `target_directory` is the directory of the extracted root
3. run `make`
4. `SimulateBasketPricer and ShapeVisitor` binary should be built into `bin` directory now

## Running the ShapeVisitor
`ShapeVisitor` can be run without any parameters.
Its only demonstrate visitor pattern with hard coded values.

## Running the simulator
To run the simulator we need to pass a few config and data files to it.
Run with `SimulateBasketPricer path_to_basket_data.csv path_to_basket_config.csv path_to_basket_item_simulation.cfg`

# Configuration Guide
Sample configurations which works are provided in cfg/data directory.

## basket_data.csv
This is the file containing the composition of each basket, and its weighting to the basket.
If we want arithmetic average, just configure the same weight for each of the basket item for thie basket

## basket_config.csv
This file is for user specifying per basket configuration.
Right now we support delta change percentage threshold for last price and mid price
such that if threshold is breached, inform the result to standard output.

## basket_item_simulation.cfg
Required to support per instrument tick data simulation. See below for details.

Sample configuration:
```
BI01
poisson_distribution,3
uniform_real_distribution,37,38
uniform_real_distribution,0,1
uniform_int_distribution,1,15
30
```

We support flexible generation of tick data as per user defined model specified above.

First line is the instrument symbol name, align with the Basket Item ID.

Second line specifies the model of tick event frequency.
In this sample we generate next tick event would be an average of 3 clock ticks later
based on Poisson distribution.

Third line is the model for generating initial bid ask prices.
In this sample we generate initial bid ask prices in the range of 37 to 38.

Fourth line is the model how we want to drive the direction of the stock.
Direction of stock will go down if the distribution generate a value less than 0.5, or go up otherwise.
In this sample with `uniform_real_distribution,0,1`, the stock will maintain roughly the same possibility
going to either direction on each tick.
As an example, if user want to model the symbol more likely going up than down, a model like
`uniform_real_distribution,0.3,1` can be employed.

Fifth line is the model specifying how many ticks it would move each time.
The distribution should be generating a range of integers.

The sixth lline is number of max tick diff between bid and ask. This is to avoid having bid ask spread
drifting too far apart. Regeneration automatically applies if the spread goes beyond the max tick diff. 

The configuration goes on and is required for each basket item.

#### Supported Random Distributions
Currently only these distributions are supported
(1) `possion_distribution` that takes 1 integer argument - mean
(2) `uniform_int_distribution` that takes 2 integer arguments - [min, max)
(3) `uniform_real_distribution` that takes 2 real number arguments - [min, max)
(4) `normal_distribution` takes 2 real number arguments - mean, standard_deviation

Other distributions require additional code changes but its highly extensible. 

# BasketPricer Design
This design aims at segregating the market data simulation with the BasketPricer. 
That is BasketPricer can be used with real market data provider,
or unit testing with mocked market data implementation. 

BasketPricer monitors each tick update, it does
- find out the instrument id
- retrieves the latest instrument price with the index based id
- compute the price delta from previous record, and update it with latest price
- for each basket that is ready, check if this instrument has a weight > 0, if so
  (1) compute the weighted delta of the basket price
  (2) update the basket accordingly
  (3) check if there is a threshold breach as per configuration

A basket is said to be ready if all basket component instruments have bid, ask and last prices published.

To achieve minimal latency with standard library only tools, vector is employed for better cache proximity.
In addition, since threshold breach print out to standard output is fairly time consuming, this design
employ another thread to dispatch the message.

Within the fast path its locking the mutex, insert the message fields into vector.
Within the thread that consume the message buffers, it pre-allocate an empty buffer and swap it with the
main thread upon message arrival, and process all outstanding messages accordingly.
This is just a fast pointer swap internally.

TODO list:
- In usual circumstances unit test cases should be written first/altogether. 
Unfortunately in this exercise only fully manually test were done while writing the code due to time constraints.
