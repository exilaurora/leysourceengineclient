#pragma once

/**
 * @file vector.h
 * @brief Простая структура 3D-вектора и вспомогательные операции.
 *
 * Используется для хранения координат и углов (typedef QAngle).
 */

struct Vector
{
	float x, y, z;

	inline Vector()
	{
		Zero();
	}
	inline Vector(float flin)
	{
		x = flin;
		y = flin;
		z = flin;
	}

	inline Vector(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}

	/** Установить вектор в (0,0,0). */
	inline void Zero()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	/** Инициализировать компоненты вектора. */
	inline void Init(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}
	float& operator[](int a)
	{
		if (a == 0)
		{
			return x;
		}

		if (a == 1)
		{
			return y;
		}

		if (a == 2)
		{
			return z;
		}

		return x;

	}

	float operator[](int a) const
	{
		if (a == 0)
		{
			return x;
		}

		if (a == 1)
		{
			return y;
		}

		if (a == 2)
		{
			return z;
		}

		return x;

	}
};

typedef Vector QAngle;