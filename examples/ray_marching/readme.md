
# ray marching in book
absorption:
$$\mathrm{d} L_o(p,\omega) = -\sigma_a (p,\omega) L_i (p,-\omega) \mathrm{d} t \quad ([a]=[\mathrm{m}^{-1}])$$

emission:
$$\mathrm{d} L_o (p,\omega) = \sigma_a L_e (p,\omega) \mathrm{d} t$$

out scattering:
$$\mathrm{d} L_o(p,\omega) = - \sigma_s (p,\omega) L_i(p,-\omega)\mathrm{d} t$$

nomen: total reduction (attenuation) $\sigma_t = \sigma_a + \sigma_s$,
albedo(transmitance) $\rho = \frac{\sigma_s}{\sigma_t}$

# ray marching in example code
ABSORPTION_COEFFICIENT: $\sigma=0.5$, fogdensity: $\rho$,
marchSize: $\Delta t$

opaqueVisibility: 
$$\Delta \ln \mathrm{transmittance} = \sigma \rho \Delta t$$

$$\therefore \mathrm{transmittance} = e^{\int \sigma \rho \mathrm{d} t}$$

let $T=e^{-\sigma\rho\Delta t}$

absorptionFromMarch: 
$$\Delta \mathrm{transmittance}\; \approx \mathrm{transmittance}\;(1-e^{-\sigma\rho\Delta t})
\approx \mathrm{transmittance}\; \sigma\rho\Delta t$$

# from chat

$$\mathrm{radiance}=\int_0^D T(0,t) L_s (t) \mathrm{d} t$$

$$\frac{\mathrm{d} T(0,t)}{\mathrm{d} t} = T(0,t) \sigma (t) = T(0,t) \sigma_t \rho(t)$$

$$L_s (t) = \mathrm{LightCache.sample}(t)\;\cdot \; \mathrm{phase}(\cos\langle\mathrm{lightDir},\mathrm{viewDir}  \rangle) \; \cdot \; \rho \sigma_s$$

Henyey-Grestien:
$$P(\cos\theta) = \frac{1}{4\pi} \frac{1-g^2}{(1+g^2 - 2 g\cos \theta)^{3/2}}\quad (\text{for cloud,}\; g=0.8)$$

