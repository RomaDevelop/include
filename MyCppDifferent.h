//---------------------------------------------------------------------------
#ifndef MyCppDifferent_H
#define MyCppDifferent_H
//---------------------------------------------------------------------------
class MyCppDifferent
{
public:
    static string ToDiapasons(vector<int> vect)
    {
	string resOff;
	if(vect.size())
	{
	    resOff = to_string(vect[0]);
	    int interval = 1;
	    for(int i=1; i<(int)vect.size(); i++)
	    {
		if(vect[i] - vect[i-1] == 1)
		{
		    if(resOff[resOff.size()-1] != '-') resOff += "-";
		    interval++;

		}
		else
		{
		    if(resOff[resOff.size()-1] == '-')
		    {
			if(interval == 2)
			{
			    resOff[resOff.size()-1] = ',';
			    resOff += " ";
			}
			resOff += to_string(vect[i-1]) + ", " + to_string(vect[i]);
		    }
		    else resOff += ", " + to_string(vect[i]);
		    interval = 1;
		}
	    }

	    if(resOff[resOff.size()-1] == '-') resOff += to_string(vect.back());
	}
	return resOff;
    }
};
#endif
