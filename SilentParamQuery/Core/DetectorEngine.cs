namespace SilentParamQuery.Core
{
    public class DetectorEngine
    {
        private readonly DieScanner _dieScanner;
        private readonly PeScanner _peScanner;

        public bool IsDieAvailable
        {
            get { return _dieScanner.IsAvailable; }
        }

        public string DiePath
        {
            get { return _dieScanner.DiePath; }
        }

        public DetectorEngine()
        {
            _dieScanner = new DieScanner();
            _peScanner = new PeScanner();
        }

        public ScanResult Scan(string filePath)
        {
            // 优先使用DIE
            if (_dieScanner.IsAvailable)
            {
                var dieResult = _dieScanner.Scan(filePath);
                if (dieResult.Success)
                    return dieResult;

                // DIE失败，尝试PE扫描
                var peResult = _peScanner.Scan(filePath);
                if (peResult.Success)
                    return peResult;

                // 两者都失败，返回DIE的错误信息
                dieResult.ErrorMessage += " (PE扫描也未命中)";
                return dieResult;
            }

            // DIE不可用，使用PE扫描
            return _peScanner.Scan(filePath);
        }
    }
}
