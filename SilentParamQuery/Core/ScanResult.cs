using System.Collections.Generic;

namespace SilentParamQuery.Core
{
    public class SilentParam
    {
        public string Switch { get; set; }
        public string Description { get; set; }
        public bool IsPrimary { get; set; }

        public SilentParam(string sw, string desc, bool isPrimary = false)
        {
            Switch = sw;
            Description = desc;
            IsPrimary = isPrimary;
        }

        public override string ToString()
        {
            return $"{Switch,-30} {Description}";
        }
    }

    public class ScanResult
    {
        public string FilePath { get; set; }
        public string FileName { get; set; }
        public string InstallerType { get; set; }
        public string InstallerFullName { get; set; }
        public string Version { get; set; }
        public string DetectedBy { get; set; }
        public double Confidence { get; set; }
        public List<SilentParam> Params { get; set; }
        public string SilentCommand { get; set; }
        public bool Success { get; set; }
        public string ErrorMessage { get; set; }

        public ScanResult()
        {
            Params = new List<SilentParam>();
            Success = false;
            Confidence = 0;
        }

        public string GetConfidenceStars()
        {
            if (Confidence >= 0.9) return "★★★★★";
            if (Confidence >= 0.7) return "★★★★☆";
            if (Confidence >= 0.5) return "★★★☆☆";
            if (Confidence >= 0.3) return "★★☆☆☆";
            return "★☆☆☆☆";
        }
    }
}
