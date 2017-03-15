# mcmd
This is a Monte Carlo / Molecular Dynamics Simulation software.

--> MC Currently fully supporting NPT,NVT,NVE,uVT ensembles.  
--> MD supporting NVT, basically.  
--> POLARIZATION IS NOT WORKING so avoid using  
    potential_form   ljespolar  

PRE-COMPILED EXECUTABLE WORKS WITH THE FOLLOWING COMPILERS:  
    -> gcc compiler 6.2.0 (circe)  
    -> gcc compiler 4.9.3 (stampede)  

To compile:  (in src dir)
g++ main.cpp -lm -o ../t -I. -std=c++11  

To run  
./t myinput.inp  
  
------------------------------------------
  
TODO

-> for some reason my RD energy is 0.001% off from MPMC every time. Maybe self energy is diff?
-> allow 0 sorbate molecules in uvt
-> make pair lists for MC to run faster. 
-> add input for easy model selection e.g. co2*, co2, h2_bssp, etc
-> make averages update every step instead of every corrtime..
    -> fix SD's maybe?
-> make universal unit-cell  
    -> NEED TO CHECK IF 4TH POINT OF PLANE MAKES THE SAME PLANE AS PREVIOUS 3 BEFORE MOVING ON
-> REDUCE CHARGE CALCULATIONS BY CONVERTING TO AU BEFOREHAND AND CONVERTING BACK IN OUTPUT
-> make correct pressure calculator (Frenkel method, check MPMC to compare)  
-> make correct thermostat for NVT dynamics  
-> include more-than-static polarization energy  
	-> right now I just use V = -0.5 sum{u.E}  
-> add rotation to MD displacements  
    -> Added but a bit dysfunctional.   
    -> maybe torque is calc'd wrong. should be local force on atoms or something  
-> Use GPU for MD force calculations? (add option)  
    -> Use GPU for polarization routine (later)  
-> Implement Phast2 model?  
