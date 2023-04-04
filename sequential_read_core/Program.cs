using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Threading.Tasks;

namespace sequential_read_core
{
	class Program
	{
		static string DataPath
		{
			get
			{
				string data = Path.GetDirectoryName(Assembly.GetAssembly(typeof(Program)).Location);
				data = Path.GetDirectoryName(data);
				data = Path.Combine(data, "Data","berlin_infos.dat");
				return data;
			}
		}
		internal static string empty = "";
		class Item
		{
			public Item(string[] items)
			{
				CITY = items[0];
				STREET = items[1];
				HOUSE = items[2];
				ZIP = items[3];
				URBANUNIT = items[4];
				OLDNAME = items[5];
				DISTRICT = items[6];
				LATITUDE = double.Parse(items[7]);
				LONGITUDE = double.Parse(items[8]);
			}

			public string CITY;
			public string STREET;
			public string HOUSE;
			public string ZIP;
			public string URBANUNIT;
			public string OLDNAME;
			public string DISTRICT;
			public double LATITUDE=0;
			public double LONGITUDE=0;
		}
	
		

		static void Main(string[] args)
		{
			Console.WriteLine(DataPath);
			Stopwatch stopwatch = new Stopwatch();
			stopwatch.Start();
			string[] lines = File.ReadAllLines(DataPath, System.Text.Encoding.Default);
			List<Item> data = new List<Item>( new Item[lines.Length] );

			Parallel.For(0, lines.Length, index => {
				string[] items = lines[index].Split(';');
				data[index] = new Item(items);
			} );

			//data.Sort( (l,r) => { return l.STREET.CompareTo(r.STREET); });

			stopwatch.Stop();
			Console.WriteLine( $"{stopwatch.ElapsedMilliseconds}ms" );
			Console.ReadKey(true);
		}
	}
}
