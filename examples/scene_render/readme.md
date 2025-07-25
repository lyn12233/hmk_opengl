# usage of blender

### add objects

`Shift+A` 

### move viewpoint

`Mouse.Middle` or drag axis panel(on top-right)

# BRDF (bidirectional reflecance distribution function)

definition: 
$$f_r(\vec i , \vec n ;\vec o) = \frac 1 {L_i \cos \theta} \frac{\mathrm d L_o}{\mathrm d \omega_i}$$

where: $\vec i$: incident light dir, $\vec o$: out light dir, $\vec n$: surface normal, $\omega, \theta$: angle

ideal microfacet reflection:
$$f_s^m(\vec i , \vec n ; \vec o) = F(\vec i , \vec n ) \frac{\delta(\vec o , \hat{\vec o})}{|\vec o \cdot \vec n|} \\= F(\vec i , \vec n) \frac{\delta(\vec h , \vec n)}{4(\vec i \cdot \vec h)^2} \quad (\vec h = (\vec i + \vec o)/|\vec i + \vec o|)$$
with $\delta$ suffices $\int_{\Omega} \delta \, \mathrm d \omega =1$

BRDF:
$$f_r(\vec i , \vec n ; \vec o) = \frac{F(\vec i , \vec h) G(\vec i , \vec o , \vec h)D(\vec h)}{4|\vec i \cdot \vec n|\cdot|\vec o \cdot \vec n|}$$
where F: fresnel term
$$F(\cos\theta)=\frac 1 2 (\frac{g-c}{g+c})^2\left( 1+ (\frac{cg+c^2-1}{cg-c^2+1})^2 \right) \\ \quad g=\sqrt{\frac{n_2^2}{n_1^2}-1+c^2}$$


G: shadow masking function

D: normal distribution function:
