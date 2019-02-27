/** @file thermodynamics.h Documented includes for thermodynamics module */

#ifndef __THERMODYNAMICS__
#define __THERMODYNAMICS__

#include "background.h"
//#include "arrays.h"
//#include "helium.h"
//#include "hydrogen.h"
#include "evolver_ndf15.h"
#include "evolver_rkck.h"
#include "wrap_hyrec.h"
#include "wrap_recfast.h"

/**
 * List of possible recombination algorithms.
 */

enum recombination_algorithm {
  recfast,
  hyrec
};

/**
 * List of possible reionization schemes.
 */

enum reionization_parametrization {
  reio_none, /**< no reionization */
  reio_camb,  /**< reionization parameterized like in CAMB */
  reio_bins_tanh,  /**< binned reionization history with tanh inteprolation between bins */
  reio_half_tanh,  /**< half a tanh, instead of the full tanh */
  reio_many_tanh,  /**< similar to reio_camb but with more than one tanh */
  reio_inter       /**< linear interpolation between specified points */
};

/**
 * Is the input parameter the reionization redshift or optical depth?
 */

enum reionization_z_or_tau {
  reio_z,  /**< input = redshift */
  reio_tau /**< input = tau */
};

/**
 * Two useful smooth step functions, for smoothing transitions in recfast.
 */

#define f1(x) (-0.75*x*(x*x/3.-1.)+0.5)  /**< goes from 0 to 1 when x goes from -1 to 1 */
#define f2(x) (x*x*(0.5-x/3.)*6.)        /**< goes from 0 to 1 when x goes from  0 to 1 */

/**
 * All thermodynamics parameters and evolution that other modules need to know.
 *
 * Once initialized by thermodynamics_init(), contains all the necessary information on the thermodynamics, and in particular, a
 * table of thermodynamical quantities as a function of the redshift, used for interpolation in other modules.
 */

struct thermo
{
  /** @name - input parameters initialized by user in input module (all other quantities are computed in this module, given these parameters
   *   and the content of the 'precision' and 'background' structures) */

  //@{

  double YHe;  /**< \f$ Y_{He} \f$: primordial helium fraction */

  enum recombination_algorithm recombination; /**< recombination code */

  enum reionization_parametrization reio_parametrization; /**< reionization scheme */

  enum reionization_z_or_tau reio_z_or_tau; /**< is the input parameter the reionization redshift or optical depth? */

  double tau_reio; /**< if above set to tau, input value of reionization optical depth */

  double z_reio;   /**< if above set to z,   input value of reionization redshift */

  short compute_cb2_derivatives; /**< do we want to include in computation derivatives of baryon sound speed? */

  short compute_damping_scale; /**< do we want to compute the simplest analytic approximation to the photon damping (or diffusion) scale? */

  /** parameters for reio_camb */

  double reionization_width; /**< width of H reionization */

  double reionization_exponent; /**< shape of H reionization */

  double helium_fullreio_redshift; /**< redshift for of helium reionization */

  double helium_fullreio_width; /**< width of helium reionization */

  /** parameters for reio_bins_tanh */

  int binned_reio_num; /**< with how many bins do we want to describe reionization? */

  double * binned_reio_z; /**< central z value for each bin */

  double * binned_reio_xe; /**< imposed \f$ X_e(z)\f$ value at center of each bin */

  double binned_reio_step_sharpness; /**< sharpness of tanh() step interpolating between binned values */

    /** parameters for reio_many_tanh */

  int many_tanh_num; /**< with how many jumps do we want to describe reionization? */

  double * many_tanh_z; /**< central z value for each tanh jump */

  double * many_tanh_xe; /**< imposed \f$ X_e(z)\f$ value at the end of each jump (ie at later times)*/

  double many_tanh_width; /**< sharpness of tanh() steps */

    /** parameters for reio_inter */

  int reio_inter_num; /**< with how many jumps do we want to describe reionization? */

  double * reio_inter_z; /**< discrete z values */

  double * reio_inter_xe; /**< discrete \f$ X_e(z)\f$ values */

  /** parameters for energy injection */

  double annihilation;           /**< parameter describing CDM annihilation (f <sigma*v> / m_cdm, see e.g. 0905.0003) */
  double annihilation_variation; /**< if this parameter is non-zero, the function F(z)=(f <sigma*v>/m_cdm)(z) will be a parabola in
				      log-log scale between zmin and zmax, with a curvature given by annihlation_variation (must be
				      negative), and with a maximum in zmax; it will be constant outside this range */
  double annihilation_z;         /**< if annihilation_variation is non-zero, this is the value of z at which the parameter annihilation is defined, i.e.
			              F(annihilation_z)=annihilation */
  double annihilation_zmax;      /**< if annihilation_variation is non-zero, redshift above which annihilation rate is maximal */
  double annihilation_zmin;      /**< if annihilation_variation is non-zero, redshift below which annihilation rate is constant */
  double annihilation_f_halo;    /**< takes the contribution of DM annihilation in halos into account*/
  double annihilation_z_halo;    /**< characteristic redshift for DM annihilation in halos*/

  short has_on_the_spot;         /**< flag to specify if we want to use the on-the-spot approximation **/

  double decay;                  /**< parameter describing CDM decay (f/tau, see e.g. 1109.6322)*/

  //@}

  /** @name - all indices for the vector of thermodynamical (=th) quantities stored in table */

  //@{

  int index_th_xe;            /**< ionization fraction \f$ x_e \f$ */
  int index_th_dkappa;        /**< Thomson scattering rate \f$ d \kappa / d \tau\f$ (units 1/Mpc) */
  int index_th_tau_d;         /**< Baryon drag optical depth */
  int index_th_ddkappa;       /**< scattering rate derivative \f$ d^2 \kappa / d \tau^2 \f$ */
  int index_th_dddkappa;      /**< scattering rate second derivative \f$ d^3 \kappa / d \tau^3 \f$ */
  int index_th_exp_m_kappa;   /**< \f$ exp^{-\kappa} \f$ */
  int index_th_g;             /**< visibility function \f$ g = (d \kappa / d \tau) * exp^{-\kappa} \f$ */
  int index_th_dg;            /**< visibility function derivative \f$ (d g / d \tau) \f$ */
  int index_th_ddg;           /**< visibility function second derivative \f$ (d^2 g / d \tau^2) \f$ */
  int index_th_Tb;            /**< baryon temperature \f$ T_b \f$ */
  int index_th_cb2;           /**< squared baryon sound speed \f$ c_b^2 \f$ */
  int index_th_dcb2;          /**< derivative wrt conformal time of squared baryon sound speed \f$ d [c_b^2] / d \tau \f$ (only computed if some non-minimal 
                                   tight-coupling schemes is requested) */
  int index_th_ddcb2;         /**< second derivative wrt conformal time of squared baryon sound speed  \f$ d^2 [c_b^2] / d \tau^2 \f$ (only computed if some 
                                   non0-minimal tight-coupling schemes is requested) */
  int index_th_rate;          /**< maximum variation rate of \f$ exp^{-\kappa}\f$, g and \f$ (d g / d \tau) \f$, used for computing integration step in perturbation module */
  int index_th_r_d;           /**< simple analytic approximation to the photon comoving damping scale */
  int th_size;                /**< size of thermodynamics vector */

  //@}

  /** @name - thermodynamics interpolation tables */

  //@{

  int tt_size; /**< number of lines (redshift steps) in the tables */
  double * z_table; /**< vector z_table[index_z] with values of redshift (vector of size tt_size) */
  double * tau_table; /**< vector tau_table[index_tau] with values of conformal time (vector of size tt_size) */
  double * thermodynamics_table; /**< table thermodynamics_table[index_z*pth->tt_size+pba->index_th] with all other quantities (array of size th_size*tt_size) */

  //@}

  /** @name - table of their second derivatives, used for spline interpolation */

  //@{

  double * d2thermodynamics_dz2_table; /**< table d2thermodynamics_dz2_table[index_z*pth->tt_size+pba->index_th] with values of \f$ d^2 t_i / dz^2 \f$ (array of size th_size*tt_size) */

  //@}


  /** @name - characteristic quantities like redshift, conformal time and sound horizon at recombination */

  //@{

  double z_rec;   /**< z at which the visibility reaches its maximum (= recombination redshift) */
  double tau_rec; /**< conformal time at which the visibility reaches its maximum (= recombination time) */
  double rs_rec;  /**< comoving sound horizon at recombination */
  double ds_rec;  /**< physical sound horizon at recombination */
  double ra_rec;  /**< conformal angular diameter distance to recombination */
  double da_rec;  /**< physical angular diameter distance to recombination */
  double rd_rec;  /**< comoving photon damping scale at recombination */
  double z_d;     /**< baryon drag redshift */
  double tau_d;   /**< baryon drag time */
  double ds_d;    /**< physical sound horizon at baryon drag */
  double rs_d;    /**< comoving sound horizon at baryon drag */
  double tau_cut; /**< at at which the visibility goes below a fixed fraction of the maximum visibility, used for an approximation in perturbation module */
  double angular_rescaling;   /**< [ratio ra_rec / (tau0-tau_rec)]: gives CMB rescaling in angular space relative to flat model (=1 for curvature K=0) */
  double tau_free_streaming;  /**< minimum value of tau at which sfree-streaming approximation can be switched on */

  //@}

  /** @name - initial conformal time at which thermodynamical variables have been be integrated */

  //@{

  double tau_ini; /**< initial conformal time at which thermodynamical variables have been be integrated */

  //@}

/** @name - total number density of electrons today (free or not) */

  //@{

  double n_e; /**< total number density of electrons today (free or not) */

  //@}

  /**
   *@name - some flags needed for thermodynamics functions
   */

  //@{

  short inter_normal;  /**< flag for calling thermodynamics_at_z and find position in interpolation table normally */
  short inter_closeby; /**< flag for calling thermodynamics_at_z and find position in interpolation table starting from previous position in previous call */

  //@}

  /** @name - technical parameters */

  //@{

  short thermodynamics_verbose; /**< flag regulating the amount of information sent to standard output (none if set to zero) */

  ErrorMsg error_message; /**< zone for writing error messages */

  //@}

};

struct thermo_heating_parameters {

  double annihilation;           /**< parameter describing CDM annihilation (f <sigma*v> / m_cdm, see e.g. 0905.0003) */
  double annihilation_variation; /**< if this parameter is non-zero, the function F(z)=(f <sigma*v>/m_cdm)(z) will be a parabola in
                                      log-log scale between zmin and zmax, with a curvature given by annihlation_variation (must be
                                      negative), and with a maximum in zmax; it will be constant outside this range */
  double annihilation_z;         /**< if annihilation_variation is non-zero, this is the value of z at which the parameter annihilation is defined, i.e.
                                      F(annihilation_z)=annihilation */
  double annihilation_zmax;      /**< if annihilation_variation is non-zero, redshift above which annihilation rate is maximal */
  double annihilation_zmin;      /**< if annihilation_variation is non-zero, redshift below which annihilation rate is constant */
  double annihilation_f_halo;    /**< takes the contribution of DM annihilation in halos into account*/
  double annihilation_z_halo;    /**< characteristic redshift for DM annihilation in halos*/

  short has_on_the_spot;         /**< flag to specify if we want to use the on-the-spot approximation **/

  double decay;                  /**< parameter describing CDM decay (f/tau, see e.g. 1109.6322)*/
};

/**
 * temporary  parameters and workspace passed to the thermodynamics_derivs function
 */

struct thermo_vector {

  int tv_size;          /**< size of thermo vector */

  int index_x_H;        /**< index for hydrogen fraction in y */
  int index_x_He;       /**< index for helium fraction in y */
  int index_Tmat;       /**< index for matter temperature fraction in y */

  double * y;           /**< vector of quantities to be integrated */
  double * dy;          /**< time-derivative of the same vector */

  int * used_in_output; /**< boolean array specifying which quantities enter in the calculation of output functions */

};

struct thermo_diffeq_workspace {

  struct thermo_vector * tv; /**< pointer to vector of integrated quantities and their time-derivatives */
  struct thermohyrec * phyrec;
  struct thermorecfast * precfast;

  double x_H;  /**< Hydrogen ionization fraction */
  double x_He; /**< Helium ionization fraction */
  double x;    /**< Electron ionization fraction */
  double dx_H;
  double dx_He;
  double dx;

  double Tmat;
  double dTmat;

  int index_ap_brec; /**< index for approximation before recombination */
  int index_ap_He1;  /**< index for 1st He-recombination (HeIII) */
  int index_ap_He1f; /**< index for approximation after 1st He recombination before 2nd */
  int index_ap_He2;  /**< index for start of 2nd He-recombination (HeII) */
  int index_ap_H;    /**< index for start of H-recombination (HI) */
  int index_ap_frec; /**< index for full recombination */
  int index_ap_reio; /**< index for reionization */
  int index_ap_reio_hyrec; /**< index for reionization with HyRec*/

  int ap_current;     /** current fixed approximation scheme index */
  int ap_size;        /**< number of approximation intervals used during evolver loop */
  int ap_size_loaded; /**< number of all approximations  */

  double * ap_z_limits;       /**< vector storing ending limits of each approximation */
  double * ap_z_limits_delta; /**< vector storing smoothing deltas of each approximation */

  int require_H;
  int require_He;

};

struct thermo_workspace {

  struct thermo_diffeq_workspace * ptdw;
  struct thermo_reionization_parameters * ptrp;
  struct thermo_heating_parameters * pthp;

  /* Parameters derived directly from the differential equation */
  int index_re_z;          /**< redshift \f$ z \f$ */
  int index_re_xe;         /**< ionization fraction \f$ x_e \f$ */
  int index_re_Tb;         /**< baryon temperature \f$ T_b \f$ */
  int index_re_cb2;        /**< squared baryon sound speed \f$ c_b^2 \f$ */
  int index_re_dkappadtau; /**< Thomson scattering rate \f$ d \kappa / d \tau\f$ (units 1/Mpc) */
  int index_re_dkappadz;   /**< Thomson scattering rate with respect to redshift \f$ d \kappa / d z\f$ (units 1/Mpc) */
  int index_re_d3kappadz3; /**< second derivative of previous quantity with respect to redshift */
  int re_size;             /**< size of this vector */

  // Number of z values
  int Nz_reio;                 /**< number of redshift points of reionization during evolver loop*/
  int Nz_reco;                 /**< number of redshifts for recombination during the evolver loop */
  int Nz_reco_lin;             /**< number of redshifts linearly sampled for recombination during the evolver loop */
  int Nz_reco_log;             /**< number of redshifts logarithmically sampled for recombination during the evolver loop */
  int Nz_tot;

  // Most important and useful parameters of evolution
  double YHe;     /**< defined as in RECFAST */
  double fHe;     /**< defined as in RECFAST */
  double SIunit_H0;      /**< defined as in RECFAST */
  double SIunit_nH0;     /**< defined as in RECFAST */
  double Tcmb;
  double R_g_factor;
  double x_limit_T;

  double reionization_optical_depth; /**< reionization optical depth inferred from reionization history */
};

struct thermo_reionization_parameters{
  
  /* parameters used by reio_camb */

  int index_reio_redshift;  /**< hydrogen reionization redshift */
  int index_reio_exponent;  /**< an exponent used in the function x_e(z) in the reio_camb scheme */
  int index_reio_width;     /**< a width defining the duration of hydrogen reionization in the reio_camb scheme */
  int index_reio_xe_before; /**< ionization fraction at redshift 'reio_start' */
  int index_reio_xe_after;  /**< ionization fraction after full reionization */
  int index_helium_fullreio_fraction; /**< helium full reionization fraction inferred from primordial helium fraction */
  int index_helium_fullreio_redshift; /**< helium full reionization redshift */
  int index_helium_fullreio_width;    /**< a width defining the duration of helium full reionization in the reio_camb scheme */

  /* parameters used by reio_bins_tanh, reio_many_tanh, reio_inter */

  int reio_num_z;                /**< number of reionization jumps */
  int index_reio_first_z;        /**< redshift at which we start to impose reionization function */
  int index_reio_first_xe;       /**< ionization fraction at redshift first_z (inferred from recombination code) */
  int index_reio_step_sharpness; /**< sharpness of tanh jump */

  /* parameters used by all schemes */

  int index_reio_start;     /**< redshift above which hydrogen reionization neglected */

  double * reionization_parameters; /**< vector containing all reionization parameters necessary to compute xe(z) */
  int reio_num_params;              /**< length of vector reionization_parameters */
};

struct thermodynamics_parameters_and_workspace {

  /* structures containing fixed input parameters (indices, ...) */
  struct background * pba;
  struct precision * ppr;
  struct thermo * pth;

  /* workspace */
  struct thermo_workspace * ptw;
  double * pvecback;

};

/**************************************************************/
/* @cond INCLUDE_WITH_DOXYGEN */
/*
 * Boilerplate for C++
 */
#ifdef __cplusplus
extern "C" {
#endif

  int thermodynamics_at_z(struct background * pba,
                          struct thermo * pth,
                          double z,
                          short inter_mode,
                          int * last_index,
                          double * pvecback,
                          double * pvecthermo);

  int thermodynamics_init(struct precision * ppr,
                          struct background * pba,
                          struct thermo * pth);

  int thermodynamics_lists(struct precision * ppr, struct background* pba, struct thermo* pth, struct thermo_workspace* ptw);

  int thermodynamics_test_parameters(struct precision * ppr,
                                     struct background* pba,
                                     struct thermo * pth);

  int thermodynamics_free(struct thermo * pth);

  int thermodynamics_indices(struct thermo * pth,
                             struct thermo_workspace* ptw);

  int thermodynamics_helium_from_bbn(struct precision * ppr,
                                     struct background * pba,
                                     struct thermo * pth);

  int thermodynamics_solve_onthespot_energy_injection(struct precision * ppr,
                                                      struct background * pba,
                                                      struct thermo_workspace * ptw,
                                                      double z,
                                                      double * energy_rate,
                                                      ErrorMsg error_message);

  int thermodynamics_solve_energy_injection(struct precision * ppr,
                                            struct background * pba,
                                            struct thermo_workspace* ptw,
                                            double z,
                                            double * energy_rate,
                                            ErrorMsg error_message);

  int thermodynamics_reionization_function(double z,
                                           struct thermo * pth,
                                           struct thermo_reionization_parameters * preio,
                                           double * x,
                                           double * dx);

  int thermodynamics_solve(struct precision * ppr,
                           struct background * pba,
                           struct thermo * pth,
                           struct thermo_workspace* ptw,
                           double * pvecback);

  int thermodynamics_solve_derivs(double mz,
                                  double * y,
                                  double * dy,
                                  void * parameters_and_workspace,
                                  ErrorMsg error_message);

  int thermodynamics_x_analytic(double z,
                                struct precision * ppr,
                                struct thermo * pth,
                                struct thermo_workspace * ptw,
                                int current_ap);

  int thermodynamics_vector_init(struct precision * ppr,
                                 struct background * pba,
                                 struct thermo * pth,
                                 double z,
                                 struct thermo_workspace * ptw);

  int thermodynamics_vector_free(struct thermo_vector * tv);

  int thermodynamics_workspace_init(struct precision * ppr,
                                    struct background * pba,
                                    struct thermo * pth,
                                    struct thermo_workspace * ptw);

  int thermodynamics_workspace_free(struct thermo* pth, struct thermo_workspace * ptw);

  int thermodynamics_set_parameters_reionization(struct precision * ppr,
                                                 struct background * pba,
                                                 struct thermo * pth,
                                                 struct thermo_reionization_parameters * preio);

  int thermodynamics_reionization_evolve_with_tau(struct thermodynamics_parameters_and_workspace * tpaw,
                                                  double mz_ini,
                                                  double mz_end,
                                                  double * mz_output,
                                                  int Nz);

  int thermodynamics_reionization_get_tau(struct precision * ppr,
                                          struct background * pba,
                                          struct thermo * pth,
                                          struct thermo_workspace * ptw);


  int thermodynamics_set_approximation_limits(struct precision * ppr,
                                              struct background * pba,
                                              struct thermo * pth,
                                              struct thermo_workspace* ptw,
                                              double mz_ini,
                                              double mz_end,
                                              int* interval_number,
                                              double * interval_limit);

  int thermodynamics_solve_store_sources(double mz,
                                         double * y,
                                         double * dy,
                                         int index_z,
                                         void * thermo_parameters_and_workspace,
                                         ErrorMsg error_message);

  int thermodynamics_output_titles(
                                   struct background * pba,
                                   struct thermo *pth,
                                   char titles[_MAXTITLESTRINGLENGTH_]
                                   );

  int thermodynamics_output_data(
                                 struct background * pba,
                                 struct thermo *pth,
                                 int number_of_titles,
                                 double *data
                                 );

  int thermodynamics_solve_timescale(double z,
                                     void * thermo_parameters_and_workspace,
                                     double * timescale,
                                     ErrorMsg error_message);

  int thermodynamics_calculate_remaining_quantities(struct precision * ppr,
                                                    struct background * pba,
                                                    struct thermo* pth,
                                                    double* pvecback);

  int thermodynamics_calculate_opticals(struct precision* ppr, struct thermo* pth);

  int thermodynamics_calculate_conformal_drag_time(struct background* pba,
                                                   struct thermo* pth,
                                                   int* last_index_back,
                                                   double* pvecback);

  int thermodynamics_calculate_damping_scale(struct background* pba,
                                             struct thermo* pth,
                                             int* last_index_back,
                                             double* pvecback);

  int thermodynamics_calculate_recombination_quantities(struct precision* ppr,
                                                        struct background * pba,
                                                        struct thermo* pth,
                                                        int* last_index_back,
                                                        double* pvecback);

  int thermodynamics_calculate_drag_quantities(struct precision* ppr,
                                               struct background * pba,
                                               struct thermo* pth,
                                               int* last_index_back,
                                               double* pvecback);

  int thermodynamics_print_output(struct background* pba, struct thermo* pth);

#ifdef __cplusplus
}
#endif

/**************************************************************/

/**
 * @name some flags
 */

//@{

#define _YHE_BBN_ -1

//@}

/**
 * @name Some basic constants needed by RECFAST:
 */

//@{

#define _m_e_ 9.10938215e-31  /**< electron mass in Kg */
#define _m_p_ 1.672621637e-27 /**< proton mass in Kg */
#define _m_H_ 1.673575e-27    /**< Hydrogen mass in Kg */
#define _not4_ 3.9715         /**< Helium to Hydrogen mass ratio */
#define _sigma_ 6.6524616e-29 /**< Thomson cross-section in m^2 */

//@}

#define _RECFAST_INTEG_SIZE_ 3


/* @endcond */
/**
 * @name Some limits imposed on cosmological parameter values:
 */

//@{

#define _YHE_BIG_ 0.5      /**< maximal \f$ Y_{He} \f$ */
#define _YHE_SMALL_ 0.01   /**< minimal \f$ Y_{He} \f$ */
#define _Z_REC_MAX_ 2000.
#define _Z_REC_MIN_ 500.

//@}

#endif
