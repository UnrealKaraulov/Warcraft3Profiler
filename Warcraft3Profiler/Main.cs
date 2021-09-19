using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace Warcraft3Profiler
{
    public partial class Main : Form
    {
        public Main()
        {
            InitializeComponent();
        }

        string FileName = string.Empty;

        private void button1_Click(object sender, EventArgs e)
        {
            OpenFileDialog f = new OpenFileDialog();
            f.ShowDialog();
            FileName = f.FileName;
        }

        struct FunctionsStruct
        {
            public int address;
            public int linescount;
        }

        List<FunctionsStruct> FunctionsStructList = new List<FunctionsStruct>();

        private void button2_Click(object sender, EventArgs e)
        {
            int updatetimes = 100;
            string[] filedata = File.ReadAllLines(FileName);
            //----- (6F(\w+)
            for (int i = 0; i < filedata.Length; i++)
            {
                if (filedata[i].IndexOf("//----- (6F", StringComparison.Ordinal) == 0)
                {
                    string funcaddr = filedata[i].Remove(0, 11);
                    funcaddr = funcaddr.Remove(6);

                    i++;

                    int argcount = filedata[i].Count(x => x == ',');
                    bool isfastcall = filedata[i].IndexOf("fastcall") > -1;
                    bool iscdecl = filedata[i].IndexOf("cdecl") > -1;
                    bool noargs = filedata[i].IndexOf(funcaddr+"()") > -1;
                    i++;

                    if (!noargs )
                    {
                        continue;
                    }

                    if (argcount > 0)
                    {
                        continue;
                    }

                  
                    int funcsize = 0;
                    while (filedata[i].IndexOf("//") != 0 && i < filedata.Length)
                    {
                        i++;
                        funcsize++;
                    }
                    i--;
                    FunctionsStruct tmp = new FunctionsStruct();
                    tmp.address = int.Parse(funcaddr, System.Globalization.NumberStyles.HexNumber);
                    tmp.linescount = funcsize;
                    FunctionsStructList.Add(tmp);
                    label2.Text = (int.Parse(label2.Text) + 1).ToString();
                    updatetimes--;
                    if (updatetimes == 0)
                    {
                        updatetimes = 100;
                        label2.Update();
                        label2.Refresh();
                        this.Update();
                        this.Refresh();
                    }
               
                }

            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            List<FunctionsStruct> functionstoexport = new List<FunctionsStruct>();
            int maxvalue = 0;
            foreach (FunctionsStruct f in FunctionsStructList)
            {
                if (maxvalue < f.linescount)
                    maxvalue = f.linescount;
            }

            int needfunctions = int.Parse(textBox1.Text);

            try
            {
                File.Delete("Out.log");
            }
            catch
            {

            }

            try
            {
                File.Delete("Out.bin");
            }
            catch
            {

            }

            if (!File.Exists("Out.bin"))
            {
                File.Create("Out.bin").Close();
            }

            int updatecount = 200;

            List<byte> b = new List<byte>();

            while (needfunctions > 0)
            {
                foreach (FunctionsStruct f in FunctionsStructList)
                {
                    if (f.linescount == maxvalue)
                    {
                        needfunctions--;
                        functionstoexport.Add(f);
                        label6.Text = (int.Parse(label6.Text) + 1).ToString();
                        updatecount--;
                        if (updatecount < 0)
                        {
                            updatecount = 200;
                            label6.Update();
                            label6.Refresh();
                            this.Update();
                            this.Refresh();
                        }
                        File.AppendAllText("Out.log", "Function Game.dll+0x" + f.address.ToString("x4") + " with " + f.linescount + "lines.");

                        b.AddRange(BitConverter.GetBytes(f.address));

                    }
                }

                maxvalue--;
            }

            File.WriteAllBytes("Out.bin", b.ToArray());


        }
    }

}