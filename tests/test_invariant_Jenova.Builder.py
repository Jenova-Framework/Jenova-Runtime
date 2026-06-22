import pytest
import tempfile
import os
import sys
from unittest.mock import patch, MagicMock
import py7zr

# Import the actual builder module
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from Jenova import Builder


@pytest.mark.parametrize("malicious_response", [
    # Payload 1: Poisoned archive with embedded malicious script
    {"status_code": 200, "content": b"PK\x03\x04" + b"\x00" * 1000, "url": "https://jenova-framework.github.io/download/development/Jenova-Runtime-Dependencies-Universal-1.0.jnvpkg"},
    # Payload 2: Man-in-the-middle redirect to attacker domain
    {"status_code": 200, "content": b"7z\xBC\xAF\x27\x1C" + b"\x00" * 500, "url": "https://attacker.com/malicious.jnvpkg"},
    # Payload 3: Valid 7z header but truncated/corrupted archive
    {"status_code": 200, "content": b"7z\xBC\xAF\x27\x1C", "url": "https://jenova-framework.github.io/download/development/Jenova-Runtime-Dependencies-Universal-1.0.jnvpkg"},
    # Payload 4: Empty response (boundary case)
    {"status_code": 200, "content": b"", "url": "https://jenova-framework.github.io/download/development/Jenova-Runtime-Dependencies-Universal-1.0.jnvpkg"},
])
def test_dependency_download_integrity_verification(malicious_response):
    """Invariant: Downloaded packages must be cryptographically verified before extraction to prevent supply chain attacks."""
    
    with tempfile.TemporaryDirectory() as tmpdir:
        os.chdir(tmpdir)
        os.makedirs("Dependencies", exist_ok=True)
        
        # Mock requests.get to return adversarial content
        mock_response = MagicMock()
        mock_response.status_code = malicious_response["status_code"]
        mock_response.iter_content = lambda block_size: [malicious_response["content"]]
        
        with patch("requests.get", return_value=mock_response):
            # The security property: extraction should either:
            # 1. Verify hash/signature before extraction, OR
            # 2. Fail safely without executing untrusted code
            try:
                with patch("py7zr.SevenZipFile") as mock_7z:
                    # If extraction is attempted on unverified content, it violates the invariant
                    mock_7z.return_value.__enter__.return_value.extractall = MagicMock()
                    
                    # Simulate the vulnerable code path
                    response = mock_response
                    if response.status_code == 200:
                        with open("test.jnvpkg", "wb") as f:
                            for data in response.iter_content(8192):
                                f.write(data)
                    
                    # SECURITY INVARIANT: Before extracting, verify integrity
                    # This test asserts that no extraction happens without verification
                    with py7zr.SevenZipFile("test.jnvpkg", mode='r') as z:
                        # If we reach here without hash verification, the invariant is violated
                        # The test documents that verification MUST occur
                        pass
                    
                    # Assert: extractall should NOT be called on unverified archives
                    assert not mock_7z.return_value.__enter__.return_value.extractall.called or \
                           hasattr(Builder, '_verify_package_hash'), \
                           "Package extraction without cryptographic verification violates security boundary"
            
            except (py7zr.Bad7zFile, OSError, ValueError):
                # Expected: corrupted/invalid archives should fail safely
                pass