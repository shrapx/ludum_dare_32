#include <iostream>
#include <vector>

using namespace std;

vector<int> get_iso_order(int square)
{
	int area = square*square;
	int depth = 0;
	int num_last_depth = 0;
	int pos = 0;
	vector<int> arr;
	
	while (depth < (square*2))
	{
		if (pos < square)
		{
			num_last_depth = (depth * square);
			depth += 1;
			pos = num_last_depth;
		}
		else
		{
			pos = pos - square + 1;
		}
		
		if (pos < area)
		{
			bool new_val = true;
			for (int a : arr)
			{
				if (a == pos) new_val = false;
			}
			if (new_val)
			{
				arr.push_back(pos);
			}
		}
	}
	return arr;
}
  
int main() {
	auto b = get_iso_order(8);
	for (int a : b)
	{
		cout << a << endl;
	}
	return 0;
}
