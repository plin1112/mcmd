#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include <stdlib.h>
#include <string>
#include <string.h>

using namespace std;


/* QUANTITIES AT STARTUP */
// =======================================================
void computeInitialValues(System &system) {
    unsigned int i, c, d, j; // index ints

    // MASS OF SYSTEM
    system.stats.totalmass.value = 0.0; system.stats.frozenmass.value = 0.0;
    for (i=0; i<system.proto.size(); i++)
        system.stats.movablemass[i].value = 0.0;
	for (c=0; c<system.molecules.size();c++) {
        string thismolname = system.molecules[c].name;
		for (d=0; d<system.molecules[c].atoms.size(); d++) {
            double thismass = system.molecules[c].atoms[d].m/system.constants.cM/system.constants.NA;
			system.stats.totalmass.value += thismass; // total mass in g
		    if (!system.molecules[c].frozen) {
                for (i=0; i<system.proto.size(); i++) {
                    if (system.proto[i].name == thismolname)
                        system.stats.movablemass[i].value += thismass;
                }

            }
            else if (system.molecules[c].frozen)
                system.stats.frozenmass.value += thismass;
        }
	}

    // N_movables (sorbates, usually)
    system.constants.initial_sorbates = system.stats.count_movables;
    for (i=0; i<system.molecules.size(); i++) {
        if (system.molecules[i].frozen) continue;
        string thismolname = system.molecules[i].name;
        for (j=0; j<system.proto.size(); j++)
            if (thismolname == system.proto[j].name)
                system.stats.Nmov[j].value++;
    }

    for (i=0; i<system.proto.size(); i++)
        system.stats.Nmov[i].average = system.stats.Nmov[i].value;

    if (system.constants.dist_within_option) {
        countAtomInRadius(system, system.constants.dist_within_target, system.constants.dist_within_radius);
        system.stats.dist_within.average = system.stats.dist_within.value;
    }

	// ENERGY
    double placeholder = getTotalPotential(system); // getTotPot sends values to stat vars
            system.stats.rd.average = system.stats.rd.value;
                system.stats.lj_lrc.average = system.stats.lj_lrc.value;
                system.stats.lj_self_lrc.average = system.stats.lj_lrc.value;
                system.stats.lj.average = system.stats.lj.value;

            system.stats.es.average = system.stats.es.value;
                system.stats.es_self.average = system.stats.es_self.value;
                system.stats.es_real.average = system.stats.es_real.value;
                system.stats.es_recip.average = system.stats.es_recip.value;

        	system.stats.polar.average = system.stats.polar.value;

            system.stats.potential.average = system.stats.potential.value;
            system.stats.potential_sq.average = system.stats.potential_sq.value;
            system.constants.initial_energy = system.stats.potential.value;

	// VOLUME
	system.stats.volume.average = system.pbc.volume;
    system.stats.volume.value = system.pbc.volume;

	// DENSITY
    for (i=0; i<system.proto.size(); i++) {
	    system.stats.density[i].value = system.stats.movablemass[i].value/(system.stats.volume.value*1e-24); // that's mass in g /mL
		system.stats.density[i].average = system.stats.density[i].value;
    }

    // WT %
    for (i=0; i<system.proto.size(); i++) {
        system.stats.wtp[i].value = (system.stats.movablemass[i].value / system.stats.totalmass.value)*100;
        system.stats.wtpME[i].value = (system.stats.movablemass[i].value / system.stats.frozenmass.value)*100;
        system.stats.wtp[i].average = system.stats.wtp[i].value;
        system.stats.wtpME[i].average = system.stats.wtpME[i].value;

    }

	// COMPRESSIBILITY FACTOR Z = PV/nRT  =  atm*L / (mol * J/molK * K)
	// GOOD FOR HOMOGENOUS GASES ONLY!!
    double n_moles_sorb = system.stats.movablemass[0].value/(system.proto[0].get_mass()*1000*system.constants.NA);
	system.stats.z.value = (system.constants.pres*(system.stats.volume.value*1e-27) * 101.325 ) // PV
            / ( (n_moles_sorb) * system.constants.R  * system.constants.temp ); // over nRT
		system.stats.z.average = system.stats.z.value;


}


/* RECURRING QUANTITIES (EVERY CORRTIME) */
// ====================================================================
void computeAverages(System &system) {
    system.checkpoint("started computeAverages()");
    double t = system.stats.MCstep;
    unsigned int i,j,c,d; // indices

    // MC MOVE ACCEPT STATS
	system.stats.total_accepts = system.stats.insert_accepts + system.stats.remove_accepts + system.stats.displace_accepts + system.stats.volume_change_accepts;
	system.stats.total_attempts = system.stats.insert_attempts + system.stats.remove_attempts + system.stats.displace_attempts + system.stats.volume_attempts;
    //printf("total attempts = %i\n",system.stats.total_attempts);

    // BOLTZMANN AVERAGES
	system.stats.bf_avg = (system.stats.insert_bf_sum + system.stats.remove_bf_sum + system.stats.displace_bf_sum + system.stats.volume_change_bf_sum)/4.0/t;
	system.stats.ibf_avg = system.stats.insert_bf_sum/t;
	system.stats.rbf_avg = system.stats.remove_bf_sum/t;
	system.stats.dbf_avg = system.stats.displace_bf_sum/t;
	system.stats.vbf_avg = system.stats.volume_change_bf_sum/t;
    system.checkpoint("got BF averages");

    // ACCEPTANCE RATIO AVERAGES
    system.stats.ar_tot = (system.stats.total_attempts == 0 ) ? 0 : system.stats.total_accepts/(double)system.stats.total_attempts;
    system.stats.ar_ins = (system.stats.insert_attempts == 0 ) ? 0 : system.stats.insert_accepts/(double)system.stats.insert_attempts;
    system.stats.ar_rem = (system.stats.remove_attempts == 0 ) ? 0 :system.stats.remove_accepts/(double)system.stats.remove_attempts;
    system.stats.ar_dis = (system.stats.displace_attempts == 0) ? 0 :system.stats.displace_accepts/(double)system.stats.displace_attempts;
    system.stats.ar_vol = (system.stats.volume_attempts == 0) ? 0 : system.stats.volume_change_accepts/(double)system.stats.volume_attempts;
    system.checkpoint("got AR averages");

    // PERCENT MOVES
    system.stats.ins_perc = (system.stats.total_accepts == 0) ? 0 : system.stats.insert_accepts/(double)system.stats.total_accepts*100;
    system.stats.rem_perc = (system.stats.total_accepts == 0) ? 0 : system.stats.remove_accepts/(double)system.stats.total_accepts*100;
    system.stats.dis_perc = (system.stats.total_accepts == 0) ? 0 : system.stats.displace_accepts/(double)system.stats.total_accepts*100;
    system.stats.vol_perc = (system.stats.total_accepts == 0) ? 0 : system.stats.volume_change_accepts/(double)system.stats.total_accepts*100;

    system.checkpoint("done with boltzmann stuff.");
    // MASS OF SYSTEM
    system.stats.totalmass.value = 0.0; system.stats.frozenmass.value = 0.0;
    for (i=0; i<system.proto.size(); i++)
        system.stats.movablemass[i].value = 0.0;
	for (c=0; c<system.molecules.size();c++) {
        string thismolname = system.molecules[c].name;
		for (d=0; d<system.molecules[c].atoms.size(); d++) {
            double thismass = system.molecules[c].atoms[d].m/system.constants.cM/system.constants.NA;
			system.stats.totalmass.value += thismass; // total mass in g
		    if (!system.molecules[c].frozen) {
                for (i=0; i<system.proto.size(); i++) {
                    if (system.proto[i].name == thismolname)
                        system.stats.movablemass[i].value += thismass;
                }

            }
            else if (system.molecules[c].frozen)
                system.stats.frozenmass.value += thismass;
        }
	}

    // N_movables (sorbates, usually)
    for (i=0; i<system.proto.size(); i++) system.stats.Nmov[i].value = 0; // initialize b4 counting.
    for (i=0; i<system.molecules.size(); i++) {
        if (system.molecules[i].frozen) continue;
        string thismolname = system.molecules[i].name;
        for (j=0; j<system.proto.size(); j++)
            if (thismolname == system.proto[j].name)
                system.stats.Nmov[j].value++;
    }

    for (i=0; i<system.proto.size(); i++)
        system.stats.Nmov[i].calcNewStats();

    if (system.constants.dist_within_option) {
        countAtomInRadius(system, system.constants.dist_within_target, system.constants.dist_within_radius);
        system.stats.dist_within.calcNewStats();
    }

    // SELECTIVITY :: N / (other Ns)
    double num, denom;
    for (i=0; i<system.proto.size(); i++) {
        num = system.stats.Nmov[i].average;
        denom=0;
        for (j=0; j<system.proto.size(); j++) {
            if (i == j) continue;
            denom += system.stats.Nmov[j].average;
        }
        if (denom != 0.0) system.stats.selectivity[i].value = num/denom; // so selec. will remain unchanged if zero sorbates in system.
        system.stats.selectivity[i].calcNewStats();
    }

	// ENERGY
    system.stats.rd.calcNewStats();
        system.stats.lj.calcNewStats();
        system.stats.lj_lrc.calcNewStats();
        system.stats.lj_self_lrc.calcNewStats();
    system.stats.es.calcNewStats();
        system.stats.es_real.calcNewStats();
        system.stats.es_self.calcNewStats();
        system.stats.es_recip.calcNewStats();
    system.stats.polar.calcNewStats();
    system.stats.potential.calcNewStats();
    system.stats.potential_sq.calcNewStats();

    // Q (partition function)
    double tmp=0;
    if (system.constants.temp>0) tmp = -(system.stats.potential.value)/system.constants.temp; // K/K = unitless
    if (tmp < 10) system.stats.Q.value += exp(tmp);

system.stats.Q.value += exp(-system.stats.potential.value / system.constants.temp); // K/K = unitless

    // QST
    if (system.constants.ensemble == ENSEMBLE_UVT && system.proto.size() == 1) { // T must be fixed for Qst

        // NU (for qst)
        system.stats.NU.value = system.stats.potential.value*system.stats.count_movables;
        system.stats.NU.calcNewStats();

        // Nsq (for qst)
        system.stats.Nsq.value = system.stats.count_movables * system.stats.count_movables;
        system.stats.Nsq.calcNewStats();

        // Qst
            if (0 != system.stats.Nsq.average - system.stats.Nmov[0].average * system.stats.Nmov[0].average) {
            double qst = -(system.stats.NU.average - system.stats.Nmov[0].average * system.stats.potential.average);
            qst /= (system.stats.Nsq.average - system.stats.Nmov[0].average * system.stats.Nmov[0].average);
            qst += system.constants.temp;
            qst *= system.constants.kb * system.constants.NA * 1e-3; // to kJ/mol
                system.stats.qst.value = qst;
                system.stats.qst.calcNewStats();

            if (0 != system.stats.Nmov[0].average) {
            double qst_nvt = -system.stats.potential.average * system.constants.NA * system.constants.kb * 1e-3 / system.stats.Nmov[0].average;
                system.stats.qst_nvt.value = qst_nvt;
                system.stats.qst_nvt.calcNewStats();
            }

        }
    }

	// VOLUME
	system.stats.volume.value = system.pbc.volume;
        system.stats.volume.calcNewStats();

	// DENSITY
    for (i=0; i<system.proto.size(); i++) {
	    system.stats.density[i].value = system.stats.movablemass[i].value/(system.stats.volume.value*1e-24); // that's mass in g /mL
        system.stats.density[i].calcNewStats();
    }

    // WT % / excess adsorption
    for (i=0; i<system.proto.size(); i++) {
        system.stats.wtp[i].value = (system.stats.movablemass[i].value / system.stats.totalmass.value)*100;
        system.stats.wtpME[i].value = (system.stats.movablemass[i].value / system.stats.frozenmass.value)*100;
            system.stats.wtp[i].calcNewStats();
            system.stats.wtpME[i].calcNewStats();

            double mm = system.proto[i].mass * 1000 * system.constants.NA; // molar mass
            double frozmm = system.stats.frozenmass.value * system.constants.NA;// ""
            system.stats.excess[i].value = 1e3*(mm*system.stats.Nmov[i].average - (mm * system.constants.free_volume * system.proto[i].fugacity * system.constants.ATM2REDUCED) / system.constants.temp) /
            frozmm;  // to mg/g

        system.stats.excess[i].calcNewStats();

    }


	// COMPRESSIBILITY FACTOR Z = PV/nRT  =  atm*L / (mol * J/molK * K)
	// GOOD FOR HOMOGENOUS GASES ONLY!!
    double n_moles_sorb = system.stats.movablemass[0].value/(system.proto[0].get_mass()*1000*system.constants.NA);
	system.stats.z.value = (system.constants.pres*(system.stats.volume.value*1e-27) * 101.325 ) // PV
            / ( (n_moles_sorb) * system.constants.R  * system.constants.temp ); // over nRT
        system.stats.z.calcNewStats();

    system.checkpoint("finished computeAverages()");

    // HEAT CAPACITY, kJ/molK
    if (system.constants.ensemble != ENSEMBLE_NVE) {
        system.stats.heat_capacity.value = (system.constants.kb*system.constants.NA*1e-3)*(system.stats.potential_sq.average - system.stats.potential.average*system.stats.potential.average)/(system.constants.temp*system.constants.temp);
        system.stats.heat_capacity.calcNewStats();
    }
}


// special variation for uVT Molecular Dynamics only
void computeAveragesMDuVT(System &system) {
    system.checkpoint("started computeAveragesMDuVT()");
    unsigned int i,j,c,d;

    // MASS OF SYSTEM
    system.stats.totalmass.value = 0.0; system.stats.frozenmass.value = 0.0;
    for (i=0; i<system.proto.size(); i++)
        system.stats.movablemass[i].value = 0.0;
	for (c=0; c<system.molecules.size();c++) {
        string thismolname = system.molecules[c].name;
		for (d=0; d<system.molecules[c].atoms.size(); d++) {
            double thismass = system.molecules[c].atoms[d].m/system.constants.cM/system.constants.NA;
			system.stats.totalmass.value += thismass; // total mass in g
		    if (!system.molecules[c].frozen) {
                for (i=0; i<system.proto.size(); i++) {
                    if (system.proto[i].name == thismolname)
                        system.stats.movablemass[i].value += thismass;
                }

            }
            else if (system.molecules[c].frozen)
                system.stats.frozenmass.value += thismass;
        }
	}

    // N_movables (sorbates, usually)
    for (i=0; i<system.proto.size(); i++) system.stats.Nmov[i].value = 0; // initialize b4 counting.
    for (i=0; i<system.molecules.size(); i++) {
        if (system.molecules[i].frozen) continue;
        string thismolname = system.molecules[i].name;
        for (j=0; j<system.proto.size(); j++)
            if (thismolname == system.proto[j].name)
                system.stats.Nmov[j].value++;
    }

    for (i=0; i<system.proto.size(); i++)
        system.stats.Nmov[i].calcNewStats();

    if (system.constants.dist_within_option) {
        countAtomInRadius(system, system.constants.dist_within_target, system.constants.dist_within_radius);
        system.stats.dist_within.calcNewStats();
    }

    // SELECTIVITY :: N / (other Ns)
    double num, denom;
    for (i=0; i<system.proto.size(); i++) {
        num = system.stats.Nmov[i].average;
        denom=0;
        for (j=0; j<system.proto.size(); j++) {
            if (i == j) continue;
            denom += system.stats.Nmov[j].average;
        }
        if (denom != 0.0) system.stats.selectivity[i].value = num/denom; // so selec. will remain unchanged if zero sorbates in system.
        system.stats.selectivity[i].calcNewStats();
    }

    // QST
    if (system.constants.ensemble == ENSEMBLE_UVT && system.proto.size() == 1) { // T must be fixed for Qst

        // NU (for qst)
        system.stats.NU.value = system.stats.potential.value*system.stats.count_movables;
        system.stats.NU.calcNewStats();

        // Nsq (for qst)
        system.stats.Nsq.value = system.stats.count_movables * system.stats.count_movables;
        system.stats.Nsq.calcNewStats();

        // Qst
            if (0 != system.stats.Nsq.average - system.stats.Nmov[0].average * system.stats.Nmov[0].average) {
            double qst = -(system.stats.NU.average - system.stats.Nmov[0].average * system.stats.potential.average);
            qst /= (system.stats.Nsq.average - system.stats.Nmov[0].average * system.stats.Nmov[0].average);
            qst += system.constants.temp;
            qst *= system.constants.kb * system.constants.NA * 1e-3; // to kJ/mol
                system.stats.qst.value = qst;
                system.stats.qst.calcNewStats();

            if (0 != system.stats.Nmov[0].average) {
            double qst_nvt = -system.stats.potential.average * system.constants.NA * system.constants.kb * 1e-3 / system.stats.Nmov[0].average;
                system.stats.qst_nvt.value = qst_nvt;
                system.stats.qst_nvt.calcNewStats();
            }

        }
    }

	// VOLUME
	system.stats.volume.value = system.pbc.volume;
        system.stats.volume.calcNewStats();

	// DENSITY
    for (i=0; i<system.proto.size(); i++) {
	    system.stats.density[i].value = system.stats.movablemass[i].value/(system.stats.volume.value*1e-24); // that's mass in g /mL
        system.stats.density[i].calcNewStats();
    }

    // WT % / excess adsorption
    for (i=0; i<system.proto.size(); i++) {
        system.stats.wtp[i].value = (system.stats.movablemass[i].value / system.stats.totalmass.value)*100;
        system.stats.wtpME[i].value = (system.stats.movablemass[i].value / system.stats.frozenmass.value)*100;
            system.stats.wtp[i].calcNewStats();
            system.stats.wtpME[i].calcNewStats();

            double mm = system.proto[i].mass * 1000 * system.constants.NA; // molar mass
            double frozmm = system.stats.frozenmass.value * system.constants.NA;// ""
            system.stats.excess[i].value = 1e3*(mm*system.stats.Nmov[i].average - (mm * system.constants.free_volume * system.proto[i].fugacity * system.constants.ATM2REDUCED) / system.constants.temp) /
            frozmm;  // to mg/g

        system.stats.excess[i].calcNewStats();

    }

	// COMPRESSIBILITY FACTOR Z = PV/nRT  =  atm*L / (mol * J/molK * K)
	// GOOD FOR HOMOGENOUS GASES ONLY!!
    double n_moles_sorb = system.stats.movablemass[0].value/(system.proto[0].get_mass()*1000*system.constants.NA);
	system.stats.z.value = (system.constants.pres*(system.stats.volume.value*1e-27) * 101.325 ) // PV
            / ( (n_moles_sorb) * system.constants.R  * system.constants.temp ); // over nRT
        system.stats.z.calcNewStats();

    system.checkpoint("finished computeAveragesMDuVT()");
}



// =================  MD statistics function to be called at corrtimes =================================
double * calculateObservablesMD(System &system) { // the * is to return an array of doubles as a pointer, not just one double
	double V_total = 0.0;
    double K_total = 0.0, Klin=0, Krot=0, Ek=0.0;

    double avg_v_ALL=0;
	double T=0.0, pressure=0;
    double vsq=0., wsq=0.;
    double energy_holder=0.;
	unsigned int i,j,n,z;

    double v_sum[(int)system.proto.size()];
    double v2_sum[(int)system.proto.size()];
    int N_local[(int)system.proto.size()];


    //if (system.stats.count_movables > 0) { // DON'T BOTHER FOR NO MOVERS
    // grab fixed potential energy of system
        // from PBC (same as Monte Carlo potential)
        if (system.constants.md_pbc) {
            V_total = getTotalPotential(system);
        // from individual atomic contributions (no PBC)
        } else {
            for (i=0; i<system.molecules.size(); i++) {
                for (j=0; j<system.molecules[i].atoms.size(); j++) {
                    V_total += system.molecules[i].atoms[j].V;
                }
            }
        }
        system.stats.potential.calcNewStats();
        system.stats.Ulin.value = system.stats.potential.value;

    // KINETIC ENERGIES, VELOCITIES, ETC. BY MOLECULE TYPE
    if(system.stats.count_movables > 0) {
    for (z = 0; z<system.proto.size(); z++) {
        v2_sum[z] = 0;
        v_sum[z] = 0;
        N_local[z] = 0;
       for (i=0; i<system.molecules.size(); i++) {
            if (system.proto[z].name == system.molecules[i].name) {
                N_local[z]++;
                if (system.constants.md_mode == MD_MOLECULAR) {
                    vsq=0; wsq=0;
                    for (n=0; n<3; n++) {
                        vsq += system.molecules[i].vel[n]*system.molecules[i].vel[n];
                        wsq += system.molecules[i].ang_vel[n]*system.molecules[i].ang_vel[n];
                    }
                    v2_sum[z] += vsq;
                    v_sum[z] += sqrt(vsq);

                    energy_holder = 0.5 * system.molecules[i].mass * vsq;
                    K_total += energy_holder;
                    Klin += energy_holder;

                    if (system.constants.md_rotations) {
                        // new tensor method.
                        system.molecules[i].calc_inertia_tensor();
                        double wx = system.molecules[i].ang_vel[0];
                        double wy = system.molecules[i].ang_vel[1];
                        double wz = system.molecules[i].ang_vel[2];

                        energy_holder = 0.5 * (system.molecules[i].inertia_tensor[0]*wx*wx +
                            system.molecules[i].inertia_tensor[1]*wy*wy +
                            system.molecules[i].inertia_tensor[2]*wz*wz +
                            2*system.molecules[i].inertia_tensor[3]*wx*wy +
                            2*system.molecules[i].inertia_tensor[4]*wy*wz +
                            2*system.molecules[i].inertia_tensor[5]*wx*wz);

                        energy_holder *= system.constants.kb/1e10;

                        K_total += energy_holder; // rotational: (rad^2)*kg A^2 / fs^2
                        Krot += energy_holder;
                    } // end if rotations
                } // end if molecular motion
                else if (system.constants.md_mode == MD_ATOMIC || system.constants.md_mode == MD_FLEXIBLE) {
                    for (j=0; j<system.molecules[i].atoms.size(); j++) {
                        vsq=0;
                        for (n=0; n<3; n++)
                            vsq += system.molecules[i].atoms[j].vel[n] * system.molecules[i].atoms[j].vel[n];
                        v_sum[z] += sqrt(vsq); // sum velocities
                        v2_sum[z] += vsq;
                        energy_holder = 0.5*system.molecules[i].atoms[j].m * vsq;
                        K_total += energy_holder;
                        Klin += energy_holder;
                    } // end for atoms j in molecule i
                } // end if atomic motion
            } // end if prototype z
        } // end molecule loop
    } // end prototype loop
    } // end if > 0 movers
    // flexible "frozen" MOF
    if (system.constants.flexible_frozen) {
       // printf("hi\n");
      // ASSUME ONLY 1 FROZEN MOLECULE with ID 0
      double vsq_tmp = 0;
      for (j=0;j<system.molecules[0].atoms.size();j++) {
        vsq_tmp = 0;
        for (n=0;n<3;n++) vsq_tmp += system.molecules[0].atoms[j].vel[n] * system.molecules[0].atoms[j].vel[n];
    //    printf("atom %i : vsq = %f m = %e\n", j, vsq_tmp, system.molecules[0].atoms[j].m);
        energy_holder = 0.5*system.molecules[0].atoms[j].m * vsq_tmp;
        K_total += energy_holder;
        Klin += energy_holder;
     //   printf("K tot = %e klin = %e \n", K_total,Klin);
      }
    } // end if flexible frozen.

    T = calcTemperature(system, N_local, v2_sum);
    system.stats.temperature.value = T;
    system.stats.temperature.calcNewStats();

    for (int z=0; z<system.proto.size(); z++)
        avg_v_ALL += (v_sum[z]/N_local[z])*(N_local[z]/(double)system.stats.count_movables);


    // fix units
    //printf("k total = %f\n",K_total);
    K_total = K_total/system.constants.kb * 1e10; // convert to K
    system.stats.kinetic.value = K_total;
    system.stats.kinetic.calcNewStats();
    system.stats.kinetic_sq.value = K_total*K_total;
    system.stats.kinetic_sq.calcNewStats();

    Klin = Klin/system.constants.kb * 1e10; // ""
    system.stats.Klin.value = Klin;

    Krot = Krot/system.constants.kb * 1e10; // ""
    system.stats.Krot.value = Krot;

    system.stats.totalE.value = K_total + V_total;
    system.stats.totalE.calcNewStats();
    system.stats.totalE_sq.value = system.stats.totalE.value*system.stats.totalE.value;
    system.stats.totalE_sq.calcNewStats();

    // HEAT CAPACITY
    // Frenkel, p58
    if (system.constants.ensemble == ENSEMBLE_NVT && system.proto.size() == 1) { // kJ/molK
        system.stats.heat_capacity.value = (system.constants.kb*system.constants.NA/1000.)*(system.stats.totalE_sq.average - system.stats.totalE.average*system.stats.totalE.average)/(system.constants.temp*system.constants.temp);
        system.stats.heat_capacity.calcNewStats();
    // Frenkel, p85
    } else if (system.constants.ensemble == ENSEMBLE_NVE && system.proto.size() == 1) {
        double kb = system.constants.kb;
        double kb2 = system.constants.kb*kb;
        double kb3 = system.constants.kb*kb2;
        double T = system.stats.temperature.value;
        double T2 = system.stats.temperature.value*T;
        double N = (double)system.stats.count_movables;
        double Kflux = kb2*(system.stats.kinetic_sq.average - system.stats.kinetic.average*system.stats.kinetic.average);

        system.stats.heat_capacity.value = 4.*N/(9.*kb3*T2);
        system.stats.heat_capacity.value *= (Kflux - 3.*kb2*T2/(2.*N));
        system.stats.heat_capacity.value = 1.0/system.stats.heat_capacity.value;
        system.stats.heat_capacity.value *= system.constants.NA/1000.; // kJ/molK
        system.stats.heat_capacity.calcNewStats();
    }

    Ek = (3.0/2.0)*system.constants.temp; // 3/2 NkT, equipartition kinetic.


    // add to partition function
    double tmp=0;
    if (T>0) {
        tmp = -(K_total+V_total)/T; // K/K = unitless
        if (tmp < 10) system.stats.Q.value += exp(tmp);
        //printf("Q += exp(-(%f+%f)/%f) = %e\n", K_total,V_total,T,exp(-(K_total+V_total)/T));
    }


  //  } // end skipping if N=0
	static double output[8];
	output[0] = K_total;
    output[1] = V_total;
	output[2] = T;
    output[3] = avg_v_ALL;
    output[4] = Ek;
    output[5] = Klin;
    output[6] = Krot;
    output[7] = pressure;

    // first step
    if (system.constants.ensemble == ENSEMBLE_NVE) {
        if (system.stats.MDtime == system.constants.md_dt) {
            system.constants.md_initial_energy_NVE = K_total+V_total; // in K
        }
        system.constants.md_NVE_err = fabs((K_total+V_total)-system.constants.md_initial_energy_NVE)*system.constants.K2KJMOL; // K to kJ/mol
    }
    return output;
}
