#!/usr/bin/env python3

import ctypes
import sys
import os
import subprocess
import platform

def test_driver_loading():
    """Test loading the MongoDB ODBC driver with ctypes"""
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    driver_paths = [
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbcw.so'),
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbca.so')
    ]
    driver_paths = [os.path.abspath(path) for path in driver_paths]
    
    for driver_path in driver_paths:
        print(f"Testing driver: {os.path.basename(driver_path)}")
        print(f"Path: {driver_path}")
        
        if not os.path.exists(driver_path):
            print(f"ERROR: Driver file does not exist: {driver_path}")
            return False
            
        if not os.access(driver_path, os.R_OK):
            print(f"ERROR: Cannot read driver file: {driver_path}")
            return False
            
        try:
            lib = ctypes.CDLL(driver_path)
            print('✅ Driver loaded successfully with ctypes')
            
            try:
                sql_driver_connect = lib.SQLDriverConnect
                print('✅ SQLDriverConnect function found')
            except AttributeError:
                print('⚠️  SQLDriverConnect function not found (may be normal for some drivers)')
                
            try:
                sql_connect = lib.SQLConnect
                print('✅ SQLConnect function found')
            except AttributeError:
                print('⚠️  SQLConnect function not found (may be normal for some drivers)')
                
        except Exception as e:
            print(f'❌ Failed to load driver: {e}')
            return False
            
        print("-" * 50)
    
    return True

def test_driver_dependencies():
    """Test if required dependencies are available"""
    print("Testing driver dependencies")
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    driver_paths = [
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbcw.so'),
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbca.so')
    ]
    
    for driver_path in driver_paths:
        if not os.path.exists(driver_path):
            continue
            
        print(f"Checking dependencies for: {os.path.basename(driver_path)}")
        
        try:
            result = subprocess.run(['ldd', driver_path], capture_output=True, text=True)
            if result.returncode == 0:
                missing_deps = [line for line in result.stdout.split('\n') if 'not found' in line]
                if missing_deps:
                    print(f"❌ Missing dependencies:")
                    for dep in missing_deps:
                        print(f"  {dep.strip()}")
                    return False
                else:
                    print("✅ All dependencies found")
            else:
                print(f"⚠️  Could not check dependencies: {result.stderr}")
        except FileNotFoundError:
            print("⚠️  'ldd' command not available")
        
        print("-" * 30)
    
    return True

def test_driver_symbols():
    """Test if driver exports expected ODBC symbols"""
    print("Testing driver symbols")
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    driver_paths = [
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbcw.so'),
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbca.so')
    ]
    
    expected_symbols = [
        'SQLAllocHandle', 'SQLFreeHandle', 'SQLConnect', 'SQLDriverConnect',
        'SQLDisconnect', 'SQLExecDirect', 'SQLPrepare', 'SQLExecute',
        'SQLFetch', 'SQLGetData', 'SQLBindCol', 'SQLGetInfo'
    ]
    
    for driver_path in driver_paths:
        if not os.path.exists(driver_path):
            continue
            
        print(f"Checking symbols in: {os.path.basename(driver_path)}")
        
        try:
            lib = ctypes.CDLL(driver_path)
            found_symbols = []
            missing_symbols = []
            
            for symbol in expected_symbols:
                try:
                    getattr(lib, symbol)
                    found_symbols.append(symbol)
                except AttributeError:
                    missing_symbols.append(symbol)
            
            print(f"✅ Found {len(found_symbols)} symbols")
            if missing_symbols:
                print(f"⚠️  Missing symbols: {', '.join(missing_symbols)}")
            
        except Exception as e:
            print(f"❌ Could not load driver to check symbols: {e}")
            return False
        
        print("-" * 30)
    
    return True

def test_driver_file_properties():
    """Test driver file properties and metadata"""
    print("Testing driver file properties")
    
    script_dir = os.path.dirname(os.path.abspath(__file__))
    driver_paths = [
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbcw.so'),
        os.path.join(script_dir, '../artifacts/drivers/libmdbodbca.so')
    ]
    
    for driver_path in driver_paths:
        if not os.path.exists(driver_path):
            continue
            
        print(f"Checking properties for: {os.path.basename(driver_path)}")
        
        # File size
        file_size = os.path.getsize(driver_path)
        print(f"File size: {file_size} bytes")
        
        if file_size == 0:
            print("❌ Driver file is empty")
            return False
        
        # File permissions
        stat_info = os.stat(driver_path)
        permissions = oct(stat_info.st_mode)[-3:]
        print(f"Permissions: {permissions}")
        
        # Check if executable
        if os.access(driver_path, os.X_OK):
            print("✅ File is executable")
        else:
            print("⚠️  File is not executable")
        
        # Check file type
        try:
            result = subprocess.run(['file', driver_path], capture_output=True, text=True)
            if result.returncode == 0:
                file_type = result.stdout.strip()
                print(f"File type: {file_type}")
                if 'ELF' not in file_type and 'shared object' not in file_type:
                    print("⚠️  File may not be a valid shared library")
            else:
                print("⚠️  Could not determine file type")
        except FileNotFoundError:
            print("⚠️  'file' command not available")
        
        print("-" * 30)
    
    return True

def test_system_environment():
    """Test system environment and compatibility"""
    print("Testing system environment")
    
    print(f"Platform: {platform.platform()}")
    print(f"Architecture: {platform.machine()}")
    print(f"Python version: {sys.version}")
    
    # Check for unixODBC
    try:
        result = subprocess.run(['odbcinst', '--version'], capture_output=True, text=True)
        if result.returncode == 0:
            print(f"✅ unixODBC version: {result.stdout.strip()}")
        else:
            print("⚠️  unixODBC not found or not working")
    except FileNotFoundError:
        print("⚠️  unixODBC not installed")
    
    # Check for required libraries
    required_libs = ['libltdl.so', 'libodbcinst.so', 'libodbc.so']
    for lib in required_libs:
        try:
            ctypes.CDLL(lib)
            print(f"✅ {lib} available")
        except OSError:
            print(f"❌ {lib} not found - this may cause loading issues")
    
    print("-" * 30)
    return True

def run_all_tests():
    """Run all driver tests"""
    tests = [
        ("System Environment", test_system_environment),
        ("Driver File Properties", test_driver_file_properties),
        ("Driver Dependencies", test_driver_dependencies),
        ("Driver Loading", test_driver_loading),
        ("Driver Symbols", test_driver_symbols)
    ]
    
    results = {}
    
    for test_name, test_func in tests:
        print(f"\n{'=' * 20} {test_name} {'=' * 20}")
        try:
            results[test_name] = test_func()
        except Exception as e:
            print(f"❌ Test failed with exception: {e}")
            results[test_name] = False
    
    return results

if __name__ == "__main__":
    print("MongoDB ODBC Driver Comprehensive Test Suite")
    print("=" * 60)
    
    results = run_all_tests()
    
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    
    all_passed = True
    for test_name, passed in results.items():
        status = "✅ PASSED" if passed else "❌ FAILED"
        print(f"{test_name}: {status}")
        if not passed:
            all_passed = False
    
    if all_passed:
        print("\n✅ All tests passed successfully")
        sys.exit(0)
    else:
        print("\n❌ Some tests failed")
        sys.exit(1)
        
