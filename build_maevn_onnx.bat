@echo off
REM MAEVN ONNX Model Builder
REM Generates default instrument ONNX models

echo ========================================
echo MAEVN ONNX Model Builder
echo ========================================
echo.

REM Check if Python is available
where python >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Python not found!
    echo Please install Python 3.10+ and add it to PATH
    echo.
    pause
    exit /b 1
)

echo Python found: 
python --version
echo.

REM Check if PyTorch is installed
python -c "import torch" 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Installing PyTorch...
    python -m pip install torch --index-url https://download.pytorch.org/whl/cpu
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to install PyTorch
        pause
        exit /b 1
    )
)

REM Check if ONNX is installed
python -c "import onnx" 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Installing ONNX...
    python -m pip install onnx
    if %ERRORLEVEL% NEQ 0 (
        echo ERROR: Failed to install ONNX
        pause
        exit /b 1
    )
)

echo.
echo ========================================
echo Exporting ONNX Models...
echo ========================================
echo.

REM Run the export script
cd scripts
python export_onnx_models.py
set EXPORT_RESULT=%ERRORLEVEL%
cd ..

if %EXPORT_RESULT% NEQ 0 (
    echo.
    echo ERROR: Model export failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo ONNX models have been generated in Models\ directory
echo.
echo Generated models:
echo - Models\drums\808_ddsp.onnx
echo - Models\drums\hihat_ddsp.onnx
echo - Models\drums\snare_ddsp.onnx
echo - Models\instruments\piano_ddsp.onnx
echo - Models\instruments\synth_fm.onnx
echo.
echo Note: Vocal models must be supplied separately
echo       Place them in Models\vocals\
echo.
pause
