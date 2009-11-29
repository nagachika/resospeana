#include "m_pd.h"
#include "math.h"

/* ------------------------ resospeana~ ----------------------------- */

static t_class *resospeana_class;

typedef struct _resospeana
{
    t_object  x_obj;
    t_float   x_f;
    t_float   sr;
    t_float   freq;
    t_float   k;
    t_float   d;
    t_float   x;
    t_float   dx;
} t_resospeana;

static t_int *resospeana_perform(t_int *w)
{
    t_resospeana *obj = (t_resospeana *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_float dt = 1.0 / obj->sr;
    t_float x_new, dx_new, ddx;
    t_float pivot;
    t_float potential;

    while(n--)
    {
	pivot = *in++;
	ddx = (-1 * obj->d * obj->dx - obj->k * (obj->x - pivot));
	dx_new = obj->dx + ddx * dt;
	x_new = obj->x + dx_new * dt;
	obj->dx = dx_new;
	obj->x = x_new;
    }

    potential = (
		 (obj->k * (obj->x - pivot) * (obj->x - pivot)) +
		 (obj->dx * obj->dx)
		) / 2.0;
    potential = potential / (0.5 * obj->k);

    outlet_float(obj->x_obj.ob_outlet, log10(potential));
    //outlet_float(obj->x_obj.ob_outlet, potential);

    return (w+4);
}

static void resospeana_dsp(t_resospeana *obj, t_signal **sp)
{
    obj->sr = sp[0]->s_sr;
    dsp_add(resospeana_perform, 3, obj, sp[0]->s_vec, sp[0]->s_n);
}

static void resospeana_ft1(t_resospeana *obj, t_float f)
{
    obj->freq = f;
    obj->k = 2.0 * M_PI * f;
    obj->k *= obj->k;
}

static void *resospeana_new(t_floatarg f)
{
    t_resospeana *obj = (t_resospeana *)pd_new(resospeana_class);
    inlet_new(&obj->x_obj, &obj->x_obj.ob_pd, &s_float, gensym("ft1"));
    outlet_new(&obj->x_obj, &s_float);
    obj->sr = 44100;
    resospeana_ft1(obj, f);
    obj->d = 50;
    obj->x = 0;
    obj->dx = 0;
    obj->x_f = 0;
    return obj;
}

void
resospeana_tilde_setup(void)
{
    resospeana_class = class_new(gensym("resospeana~"),
				(t_newmethod)resospeana_new, 0,
				sizeof(t_resospeana), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(resospeana_class, t_resospeana, x_f);
    class_addmethod(resospeana_class, (t_method)resospeana_dsp, gensym("dsp"), 0);
    class_addmethod(resospeana_class, (t_method)resospeana_ft1, gensym("ft1"), A_FLOAT, 0);
}

