#pragma once


#include "hittable.h"

class material
{
public:
	virtual ~material() = default;

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const
	{
		return false;
	}
};


class lambertian : public material
{
public:
	lambertian(const color& albedo) : albedo(albedo)
	{
	}

	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
	const override
	{
		auto scatter_direction = rec.normal + random_unit_vector();

		// Catch degenerate scatter direction ����ɢ�䷽��ӽ�0�����
		if (scatter_direction.near_zero()) scatter_direction = rec.normal;


		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	}

private:
	color albedo;
};


class metal : public material
{
public:
	metal(const color& albedo) : albedo(albedo)
	{
	}

	metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}


	bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
	const override
	{
		vec3 reflected = reflect(r_in.direction(), rec.normal);
		reflected = unit_vector(reflected) + (fuzz * random_unit_vector());    // ģ������

		scattered = ray(rec.p, reflected);
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);   // ɢ�䷽��ͷ���ͬ�࣬ɢ������Ĺ����յ�
	}

private:
	color albedo;
	double fuzz;

};
